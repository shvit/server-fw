/*
 * tftpClientSession.cpp
 *
 *  Created on: 22 июн. 2021 г.
 *      Author: svv
 */

#include <string.h>
#include <unistd.h>

#include "tftpClientSession.h"
#include "tftpDataMgrFileRead.h"
#include "tftpDataMgrFileWrite.h"

#include <iostream>

namespace tftp
{

//------------------------------------------------------------------------------

ClientSession::ClientSession(pClientSettings && sett, fLogMsg new_cb):
    Logger(new_cb),
    stat_{State::need_init},
    settings_{std::move(sett)},
    local_addr_{},
    socket_{0},
    stage_{0U},
    file_size_{0U},
    error_code_{0U},
    error_message_{},
    need_break_{false},
    stopped_{false},
    file_man_{nullptr},
    srv_session_set_{false}
{
}

//------------------------------------------------------------------------------

ClientSession::ClientSession():
    ClientSession(ClientSettings::create(), nullptr)
{
}

//------------------------------------------------------------------------------

auto ClientSession::create(pClientSettings && sett, fLogMsg new_cb) -> pClientSession
{
  struct Enabler: public ClientSession
  {
    Enabler(pClientSettings && n_sett, fLogMsg n_new_cb):
      ClientSession(std::move(n_sett), n_new_cb) {};
  };

  return std::make_unique<Enabler>(std::move(sett), new_cb);
}

//------------------------------------------------------------------------------

bool ClientSession::switch_to(const State & new_state)
{
  bool ret = (stat_ == new_state);

  if(!ret)
  {
    switch(stat_)
    {
      case State::need_init:
        ret = (new_state == State::finish ) ||
              (new_state == State::request);
        break;
      case State::request:
        ret = (new_state == State::ack_options) ||
              (new_state == State::data_rx    ) ||
              (new_state == State::error_and_stop);
        break;
      case State::error_and_stop:
        ret = (new_state == State::finish);
        break;
      case State::ack_options:
        ret = (new_state == State::data_tx) ||
              (new_state == State::ack_tx ) ||
              (new_state == State::error_and_stop);
        break;
      case State::data_tx:
        ret = (new_state == State::ack_rx) ||
              (new_state == State::error_and_stop);
        break;
      case State::data_rx:
        ret = (new_state == State::ack_tx) ||
              (new_state == State::retransmit);
        break;
      case State::ack_tx:
        ret = (new_state == State::data_rx) ||
              (new_state == State::finish);
        break;
      case State::ack_rx:
        ret = (new_state == State::data_tx) ||
              (new_state == State::retransmit) ||
              (new_state == State::finish);
        break;
      case State::retransmit:
        ret = (new_state == State::data_tx) ||
              (new_state == State::ack_tx ) ||
              (new_state == State::error_and_stop);
        break;
      case State::finish: // no way to switch
        break;
    }
  }

  if(ret)
  {
    L_DBG("State: "+stat_+" -> "+new_state);
    stat_.store(new_state);
  }
  else
  {
    L_ERR("Wrong switch state: "+stat_+" -> "+new_state+"! Switch to finish");
    stat_.store(State::finish); // Skip any loop
  }

  return ret;
}

// -----------------------------------------------------------------------------

void ClientSession::set_error_if_first(
    const uint16_t e_cod,
    std::string_view e_msg)
{
  if(!was_error())
  {
    L_DBG("Register error #"+std::to_string(e_cod)+
          " '"+std::string(e_msg)+"'");
    error_code_ = e_cod;
    error_message_.assign(e_msg);
  }
  else
  {
    L_DBG("Skip error #"+std::to_string(e_cod)+
          " '"+std::string(e_msg)+"'");
  }
}

// -----------------------------------------------------------------------------

bool ClientSession::was_error() const
{
  return (error_code_ > 0U) || (error_message_.size() > 0U);
}

//------------------------------------------------------------------------------

bool ClientSession::init()
{
  L_INF("Session initialize started");

  bool ret = false;

  // Calc directory and filename
  Path loc_file_name{Path{settings_->file_local}.filename()};
  Path loc_file_path{Path{settings_->file_local}.replace_filename("")};

  if(!filesystem::is_directory(loc_file_path))
  {
    loc_file_path = filesystem::current_path();
    L_INF("Use current work directory "+loc_file_path.string());
  }

  // Create file manager
  switch(settings_->opt.request_type())
  {
    case SrvReq::read:
      file_man_ = DataMgrFileWrite::create(
          get_logger(),
          std::bind(
              & ClientSession::set_error_if_first,
              this,
              std::placeholders::_1,
              std::placeholders::_2),
              loc_file_name.string(),
              loc_file_path.string());
      break;
    case SrvReq::write:
      file_man_ = DataMgrFileRead::create(
          get_logger(),
          std::bind(
              & ClientSession::set_error_if_first,
              this,
              std::placeholders::_1,
              std::placeholders::_2),
              loc_file_name.string(),
              loc_file_path.string(),
              {});
      break;
    default: // fake
      L_ERR("Wrong request type '"+settings_->opt.request_type()+"'");
      goto RET_INIT;
  }

  // Check file manager active (with try open stream)
  if(!(ret = file_man_->open()))
  {
    file_man_.release();
    L_ERR("File stream not opened");
    goto RET_INIT;
  }

  // Local address
  local_addr_.clear();
  local_addr_.set_family(settings_->srv_addr.family());

  // Socket create
  socket_ = socket(local_addr_.family(), SOCK_DGRAM, 0);
  if ((ret = (socket_>= 0)))
  {
    L_DBG("Socket opened successful");
  }
  else
  {
    Buf err_msg_buf(1024, 0);
    L_ERR("socket() error: "+
          std::string{strerror_r(errno,
                                 err_msg_buf.data(),
                                 err_msg_buf.size())});
    cancel();
    goto RET_INIT;
  }

  // Socket bind
  if((ret = bind(
      socket_,
      (struct sockaddr *) local_addr_.data(),
      local_addr_.data_size()) != -1))
  {
    L_DBG("Bind socket successful");
  }
  else
  {
    Buf err_msg_buf(1024, 0);
    L_ERR("bind() error: "+
            std::string{strerror_r(errno,
                                   err_msg_buf.data(),
                                   err_msg_buf.size())});
    cancel();
    goto RET_INIT;
  }

RET_INIT:
  if(ret) switch_to(State::request);
  stopped_.store(!ret);
  L_INF("Session initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));
  return ret;
}

// -----------------------------------------------------------------------------

void ClientSession::close()
{
  if(file_man_.get()) file_man_->close();

  ::close(socket_);
}

// -----------------------------------------------------------------------------

void ClientSession::cancel()
{
  if(file_man_.get()) file_man_->cancel();

  ::close(socket_);
}

// -----------------------------------------------------------------------------

bool ClientSession::is_finished() const
{
  return (stat_ == State::finish);
}

// -----------------------------------------------------------------------------

auto ClientSession::run_session() -> ClientSessionResult
{
  L_INF("Running session");

  //if(!file_man_->active())
  //{
  //  L_ERR("File streams not active");
  //  cancel();
  //  stopped_ = true;
  //  return ClientSessionResult::fail_run;
  //}

  // Prepare
  bool last_blk_processed_{false};
  uint16_t retr_count{0U};
  SmBufEx local_buf{0xFFFFU};
  time_t oper_time_{0};

  auto timeout_pass = [&]()
      { return (time(nullptr) - oper_time_) < (settings_->opt.timeout() + 1); };

  auto timeout_reset = [&]()
      { oper_time_ = time(nullptr); };


  stage_ = 0U;
  while(!is_finished() && !stopped_)
  {
    switch(stat_)
    {
      case State::need_init: // ------------------------------------------------
        if(init())
        {
          if(was_error())
          {
            switch_to(State::finish);
          }
          else
          {
            switch_to(State::request);
          }
        }
        else
        {
          switch_to(State::finish);
        }
        break;

      case State::error_and_stop: // -------------------------------------------
        if(was_error())
        {
          construct_error(local_buf);
          transmit_no_wait(local_buf);
        }
        switch_to(State::finish);
        break;

      case State::request: // --------------------------------------------------
        construct_request(local_buf);
        if(transmit_no_wait(local_buf))
        {
          timeout_reset();
          if(settings_->opt.was_set_any())
          {
            switch_to(State::ack_options);
          }
          else
          {
            switch_to(State::data_rx);
          }
        }
        else // if error tx
        {
          switch_to(State::finish);
        }
        break;

      case State::ack_options: // ----------------------------------------------
        switch(receive_no_wait(local_buf))
        {
          case TripleResult::nop:
            if(!timeout_pass())
            {
              switch_to(State::retransmit);
            }
            break;
          case TripleResult::ok:
            switch(settings_->opt.request_type())
            {
              case SrvReq::read:
                switch_to(State::ack_tx);
                stage_ = 0U;
                break;
              case SrvReq::write:
                switch_to(State::data_tx);
                stage_ = 1U;
                break;
              default: // never do it
                switch_to(State::error_and_stop);
                break;
            }

            break;
          case TripleResult::fail:
            switch_to(State::error_and_stop);
            break;
        }
        break;

      case State::data_tx: // --------------------------------------------------
        {
          construct_data(local_buf);
          if(!was_error() && (local_buf.data_size() > 0U))
          {
            transmit_no_wait(local_buf);
            //++stage_;
            last_blk_processed_ = local_buf.data_size() != (block_size()+4U);

            if(is_window_close(stage_) || last_blk_processed_)
            {
              timeout_reset();
              switch_to(State::ack_rx);
            }
            else
            {
              ++stage_;
            }
          }
          else
          {
            switch_to(State::error_and_stop);
          }
        }
        break;

      case State::data_rx: // --------------------------------------------------
        switch(receive_no_wait(local_buf))
        {
          case TripleResult::nop:
            if(!timeout_pass())
            {
              switch_to(State::retransmit);
            }
            break;
          case TripleResult::ok:
            last_blk_processed_ = local_buf.data_size() != (block_size()+4U);
            if(is_window_close(stage_) || last_blk_processed_)
            {
              switch_to(State::ack_tx);
            }
            else
            {
              ++stage_;
              timeout_reset();
            }
            break;
          case TripleResult::fail:
            switch_to(State::error_and_stop);
            break;
        }

        break;

          case State::ack_tx: // ---------------------------------------------------
            construct_ack(local_buf);
            transmit_no_wait(local_buf);
            ++stage_;
            if(last_blk_processed_)
            {
              switch_to(State::finish);
            }
            else
            {
              switch_to(State::data_rx);
              timeout_reset();
            }
            break;

          case State::ack_rx: // ---------------------------------------------------
            switch(receive_no_wait(local_buf))
            {
              case TripleResult::nop:
                if(!timeout_pass())
                {
                  switch_to(State::retransmit);
                }
                break;
              case TripleResult::ok:
                if(last_blk_processed_)
                {
                  switch_to(State::finish);
                }
                else
                {
                  switch_to(State::data_tx);
                  ++stage_;
                  timeout_reset();
                }
                break;
              case TripleResult::fail:
                switch_to(State::error_and_stop);
                break;
            }
            break;

          case State::retransmit: // -----------------------------------------------
            if(++retr_count > settings_->retransmit_count)
            {
              L_WRN("Retransmit count exceeded ("+std::to_string(retr_count)+
                    "); Break session");
              switch_to(State::error_and_stop);
            }
            else
            {
              //step_back_window(stage_);
              switch(settings_->opt.request_type())
              {
                case SrvReq::unknown:
                  switch_to(State::error_and_stop);
                  break;
                case SrvReq::read:
                  switch_to(State::ack_tx);
                  break;
                case SrvReq::write:
                  switch_to(State::data_tx);
                  break;
              }
              timeout_reset();
            }
            break;









      default:
        switch_to(State::finish);
        break;
    }
    //sleep(2); break;
  } // end main loop


  //cancel();
  L_INF("Finish session");
  stopped_.store(true);
  return ClientSessionResult::ok;
}

// -----------------------------------------------------------------------------

bool ClientSession::is_window_close(const size_t & curr_stage) const
{
  return (curr_stage % settings_->opt.windowsize()) == 0U;
}

// -----------------------------------------------------------------------------

auto ClientSession::run() -> ClientSessionResult
{
  if(init())
  {
    switch_to(State::request);
  }
  else
  {
    L_ERR("Wrong run - fail init session");
    cancel();
    return ClientSessionResult::fail_init;
  }

  return run_session();
}

// -----------------------------------------------------------------------------

void ClientSession::need_break()
{
  need_break_ = true;
}

// -----------------------------------------------------------------------------

auto ClientSession::blk_num_local() const -> uint16_t
{
  return (stage_ & 0x000000000000FFFFU);
}

// -----------------------------------------------------------------------------

auto ClientSession::block_size() const -> uint16_t
{
  return settings_->opt.blksize();
}

// -----------------------------------------------------------------------------

void ClientSession::construct_request(SmBufEx & buf) const
{
  buf.clear();

  buf.push_data((uint16_t) settings_->opt.request_type());

  buf.push_data(settings_->opt.filename());

  buf.push_data(to_string(settings_->opt.transfer_mode()));

  if(settings_->opt.was_set_blksize())
  {
    std::string name{constants::name_blksize};
    std::string value{std::to_string(settings_->opt.blksize())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }
  if(settings_->opt.was_set_timeout())
  {
    std::string name{constants::name_timeout};
    std::string value{std::to_string(settings_->opt.timeout())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }
  if(settings_->opt.was_set_windowsize())
  {
    std::string name{constants::name_windowsize};
    std::string value{std::to_string(settings_->opt.windowsize())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }
  if(settings_->opt.was_set_tsize())
  {
    std::string name{constants::name_tsize};
    std::string value{std::to_string(settings_->opt.tsize())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }

  L_DBG("Construct request '"+settings_->opt.request_type()+
        "' pkt with "+std::to_string(buf.data_size())+" octets");
}

// -----------------------------------------------------------------------------

void  ClientSession::construct_error(SmBufEx & buf)
{
  if(!was_error())
  {
    error_code_ = 0U;
    error_message_ = "Undefined error";
  }

  buf.clear();

  buf.push_data((uint16_t) 5U, error_code_, error_message_);

  L_DBG("Construct error pkt #"+std::to_string(error_code_)+
        " '"+std::string(error_message_)+"'; "+
        std::to_string(buf.data_size())+" octets");
}

// -----------------------------------------------------------------------------

void ClientSession::construct_data(SmBufEx & buf)
{
  buf.clear();

  buf.push_data((uint16_t) 3U, blk_num_local());

  ssize_t ret = 0;

  if(stage_ != 0U)
  {
    ret = file_man_->read(
        buf.begin() + buf.data_size(),
        buf.begin() + buf.data_size() + block_size(),
        (stage_-1U) *  block_size());
  }

  if(ret >=0)
  {
    buf.data_size_reset(buf.data_size() + ret);

    L_DBG("Construct data pkt block "+std::to_string(stage_)+
            "; data size "+std::to_string(ret)+" bytes");
  }
  else // error prepare data
  {
    L_ERR("Error prepare data");
    set_error_if_first(0, "Failed prepare data to send");
  }
}

// -----------------------------------------------------------------------------

void ClientSession::construct_ack(SmBufEx & buf)
{
  buf.clear();

  buf.push_data((uint16_t)4U, blk_num_local());

  L_DBG("Construct ACK pkt block "+std::to_string(blk_num_local()));
}

// -----------------------------------------------------------------------------

bool ClientSession::transmit_no_wait(const SmBufEx & buf)
{
  bool ret = false;

  if(buf.data_size() > 0U)
  {
    ssize_t tx_result_size = sendto(
        socket_,
        buf.data(),
        buf.data_size(),
        0,
        settings_->srv_addr.as_sockaddr_ptr(),
        settings_->srv_addr.data_size());

    ret = (tx_result_size == (ssize_t)buf.data_size());

    if(ret) // Good send
    {
      L_DBG("Success send packet "+std::to_string(buf.data_size())+
            " octets");
    }
    else // Fail send
    {
      if(tx_result_size < 0)
      {
        L_ERR("sendto() error");
      }
      else
      {
        L_ERR("sendto() lost data error: sended "+
              std::to_string(tx_result_size)+
              " from "+std::to_string(buf.data_size()));
      }
    }
  }
  else
  {
    L_ERR("Nothing to send; prepared data size 0 bytes");
  }

  return ret && !was_error();
}

// -----------------------------------------------------------------------------

//void ClientSession::set_srv_port(uint16_t new_port)
//{
//  L_DBG("Set new session server port ("+std::to_string(new_port)+") from server reply");
//  settings_->srv_addr.set_port(new_port);
//}

// -----------------------------------------------------------------------------

auto ClientSession::receive_no_wait(SmBufEx & buf) -> TripleResult
{
  Addr rx_client;
  rx_client.data_size() = rx_client.size();

  ssize_t rx_pkt_size = recvfrom(
      socket_,
      buf.data(),
      buf.size(),
      MSG_DONTWAIT,
      rx_client.as_sockaddr_ptr(),
      & rx_client.data_size());

  if(rx_pkt_size < 0) // Any receive error? - go away
  {
    switch(errno)
    {
      case EAGAIN:  // OK
        return TripleResult::nop;
      default:
        L_ERR("Error #"+std::to_string(errno)+
                " when call recvfrom(). Break loop!");
        return TripleResult::fail;
    }
  }

  buf.data_size_reset((size_t) rx_pkt_size);

  // Extract meta info
  uint16_t rx_op  = (rx_pkt_size > 3) ? buf.get_be<uint16_t>(0U) : 0U;
  uint16_t rx_blk = (rx_pkt_size > 3) ? buf.get_be<uint16_t>(2U) : 0U;
  uint16_t rx_data_size = (rx_pkt_size > 3) ? (uint16_t)(rx_pkt_size - 4) : 0U;

  // Make debug message
  std::string rx_msg = "Rx pkt ["+std::to_string(rx_pkt_size)+" octets]";
  switch(rx_op)
  {
    case 3U: // DATA
      rx_msg.append(": DATA blk "+std::to_string(rx_blk)+
                    "; data size "+std::to_string(rx_data_size));
      break;
    case 4U: // ACK
      rx_msg.append(": ACK blk "+std::to_string(rx_blk));
      break;
    case 5U: // ERROR
      rx_msg.append(": ERROR #"+std::to_string(rx_blk)+
                    " '"+buf.get_string(4U)+"'");
      break;
    case 6U: // OPTION ACK
      rx_msg.append(": Option ACK");
      break;
    default:
      rx_msg.append(": FAKE tftp packet");
      break;
  }

  // Set server port from first server pkt
  if(!srv_session_set_)
  {
    L_DBG("Set new session server value ("+rx_client.str()+") from server reply");
    //set_srv_port(rx_client.port());
    settings_->srv_addr = rx_client;

    srv_session_set_ = true;
  }

  // Check client address is right
  if(rx_client.eqv_addr_only(settings_->srv_addr))
  {
    L_DBG(rx_msg+" from client");
  }
  else
  {
    L_DBG("SRV from settings "+settings_->srv_addr.str());
    L_DBG("SRV from packet   "+rx_client.str());
    L_WRN("Alarm! Intrusion detect from addr "+rx_client.str()+
          " with data: "+rx_msg+". Ignore pkt!");
    return TripleResult::nop;
  }

  ssize_t rx_stage = (ssize_t)stage_ +
                      (ssize_t)rx_blk - (ssize_t)blk_num_local();

  switch(rx_op)
  {
    case 3U: // DATA
      if((stat_ == State::data_rx) ||
         (stat_ == State::ack_options))
      {
        if((stat_ == State::ack_options))
        {
          L_INF("No options ack; reset all options");
          settings_->opt.reset_all();
        }

        if(rx_stage < 0)
        {
          L_WRN("Wrong Data blk! rx #"+std::to_string(rx_blk)+
                " need #"+std::to_string(blk_num_local())+
                "; calculated stage="+std::to_string(rx_stage)+". Break session!");
          set_error_if_first(0, "Error received number data block");
          return  TripleResult::fail;
        }

        if(rx_stage > (ssize_t)(stage_+1U))
        {
          L_WRN("Skip (lost) data blocks! rx #"+std::to_string(rx_blk)+
                " need #"+std::to_string(blk_num_local())+
                "; calculated stage="+std::to_string(rx_stage)+". Break session!");
          set_error_if_first(0, "Error received number data block");
          return  TripleResult::fail;
        }

        if(blk_num_local() != rx_blk)
        {
          L_INF("Switch blk #"+std::to_string(blk_num_local())+
                " -> #"+std::to_string(rx_blk));
          stage_ = (size_t) rx_stage;
        }

        ssize_t stored_data_size =  file_man_->write(
            buf.cbegin() + 2*sizeof(uint16_t),
            buf.cbegin() + rx_pkt_size,
            (stage_ - 1) * block_size());
        if(stored_data_size < 0)
        {
          L_ERR("Error from store data manager");
          set_error_if_first(0, "Error when try to store data");
          return  TripleResult::fail;
        }
        return  TripleResult::ok;
      }
      break;
    case 4U: // ACK
      if(stat_ == State::ack_rx)
      {
        if(rx_stage < 0)
        {
          L_WRN("Wrong Data ack! rx #"+std::to_string(rx_blk)+
                " need #"+std::to_string(blk_num_local())+
                "; calculated stage="+std::to_string(rx_stage)+". Break session!");
          set_error_if_first(0, "Error received number ack block");
          return  TripleResult::fail;
        }

        if(rx_stage > (ssize_t)(stage_+1U))
        {
          L_WRN("Skip (lost) data blocks! rx #"+std::to_string(rx_blk)+
                " need #"+std::to_string(blk_num_local())+
                "; calculated stage="+std::to_string(rx_stage)+". Break session!");
          set_error_if_first(0, "Error received number ack block");
          return  TripleResult::fail;
        }

        if(blk_num_local() != rx_blk)
        {
          L_INF("Switch blk #"+std::to_string(blk_num_local())+
                " -> #"+std::to_string(rx_blk));
          stage_ = (size_t) rx_stage;
        }
        return  TripleResult::ok;
      }
      break;

    case 5U: // ERROR
      L_ERR("Server reply error #" + std::to_string(rx_blk)+
                          " '"+buf.get_string(4U)+"'");
      return TripleResult::fail;

    case 6U: // OPTION ACK
      if(stat_ == State::ack_options)
      {
        Options confirm_opt;
        confirm_opt.buffer_parse_oack(
            buf,
            rx_pkt_size);

        settings_->opt.apply_oack(confirm_opt, log_);
        return  TripleResult::ok;
      }
      break;
    default: // WTF?
      break;
  }




  // Parse packet if need and do receive DATA

  // Parse packet if need aswitch_to(State::request);nd do receive ACK

  return TripleResult::nop;
}

// -----------------------------------------------------------------------------

} // namespace tftp
