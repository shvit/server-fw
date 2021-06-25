/*
 * tftpClientSession.cpp
 *
 *  Created on: 22 июн. 2021 г.
 *      Author: svv
 */

#include <string.h>
#include <unistd.h>

#include "tftpClientSession.h"
#include "tftpBase.h"

namespace tftp
{

//------------------------------------------------------------------------------

ClientSession::ClientSession(
    std::ostream * stream,
    int argc,
    char * argv[]):
    ClientSession(stream)
{
  init(argc, argv);
}

//------------------------------------------------------------------------------

ClientSession::ClientSession(std::ostream * stream):
    stat_{State::need_init},
    pstream_{stream},
    settings_{},
    local_addr_{},
    socket_{0},
    stage_{0U},
    file_in_{},
    file_out_{},
    file_size_{0U},
    error_code_{0U},
    error_message_{},
    need_break_{false},
    stopped_{false}
{
  if(pstream_) settings_.out_header(*pstream_);
}

//------------------------------------------------------------------------------

ClientSession::ClientSession():
    ClientSession(nullptr)
{
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
        ret = (new_state == State::finish        ) ||
              (new_state == State::error_and_stop) ||
              (new_state == State::request);
        break;
      case State::request:
        ret = (new_state == State::ack_options   ) ||
              (new_state == State::error_and_stop);
        break;
      case State::error_and_stop:
        ret = (new_state == State::finish);
        break;
      case State::ack_options:
        ret = (new_state == State::data_tx) ||
              (new_state == State::ack_tx );
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

// -----------------------------------------------------------------------------

void ClientSession::log(LogLvl lvl, std::string_view msg) const
{
  if((int)lvl <=settings_.verb)
  {
    if(pstream_) *pstream_ << "[" << to_string(lvl) << "] " <<  msg << std::endl;
  }
}

//------------------------------------------------------------------------------

bool ClientSession::init(
    int argc,
    char * argv[])
{
  bool ret = settings_.load_options(
      std::bind(
          & ClientSession::log,
          this,
          std::placeholders::_1,
          std::placeholders::_2),
      argc,
      argv);

  return ret && init_session();
}


//------------------------------------------------------------------------------

bool ClientSession::init_session()
{
  L_INF("Session initialize started");

  bool ret = (settings_.opt.request_type() == SrvReq::write) ||
             (settings_.opt.request_type() == SrvReq::read);
  if(!ret)
  {
    L_ERR("Wrong request type '"+settings_.opt.request_type()+"'");
  }

  // Check local file exist
  if(ret)
  {
    if(settings_.opt.request_type() == SrvReq::write)
    {
      ret=filesystem::exists(settings_.file_local);
      if(!ret)
      {
        L_ERR("Local file not exist '"+settings_.file_local+"'");
      }
    }
    else
    if(settings_.opt.request_type() == SrvReq::read)
    {
      ret=!filesystem::exists(settings_.file_local);
      if(!ret)
      {
        L_ERR("Local file already exist '"+settings_.file_local+"'");
      }
    }
  }

  // Calc file size
  if(ret)
  {
    file_size_=0U;
    if(settings_.opt.was_set_tsize())
    {
      auto tmp_val = settings_.opt.tsize();
      if(tmp_val > 0) file_size_ = tmp_val;
    }
    else
    {
      if(settings_.opt.request_type() == SrvReq::write)
      {
        ret = filesystem::exists(settings_.file_local);
        if(ret)
        {
          file_size_=filesystem::file_size(settings_.file_local);
          L_DBG("Found local file '"+settings_.file_local+
                "' size "+std::to_string(file_size_));
        }
        else
        {
          L_ERR("Not found local file '"+settings_.file_local+"'");
        }
      }
    }
  }

  // File open
  if(ret)
  {
    if(settings_.opt.request_type() == SrvReq::read)
    {
      file_out_.exceptions(file_out_.exceptions() | std::ios::failbit);
      try
      {
        file_out_.open(
            settings_.file_local,
            std::ios_base::out | std::ios::binary);
        file_out_.write(nullptr, 0U);
      }
      catch (const std::system_error & e)
      {
        ret = false;
        L_ERR(std::string{"Error: "}+e.what()+" ("+
              std::to_string(e.code().value())+")");
      }
    }
    else
    if(settings_.opt.request_type() == SrvReq::write)
    {
      auto backup_val = file_in_.exceptions();
      file_in_.exceptions(std::ios::failbit);
      try
      {
        file_in_.open(
            settings_.file_local,
            std::ios_base::in | std::ios::binary);
      }
      catch (const std::system_error & e)
      {
        ret = false;
        L_ERR(std::string{"Error: "}+e.what()+" ("+
              std::to_string(e.code().value())+")");
      }
      file_in_.close();
      file_in_.exceptions(backup_val);
      file_in_.open(
          settings_.file_local,
          std::ios_base::in | std::ios::binary);

    }
    else
    {
      ret = false;
      L_ERR("Wrong request type '"+settings_.opt.request_type()+ "'");
    }
  }

  if(ret)
  {
    // Local address
    local_addr_.clear();
    local_addr_.set_family(settings_.srv_addr.family());

    // Socket
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
    }
  }

  // Bind socket
  if(ret)
  {
    ret = bind(
        socket_,
        (struct sockaddr *) local_addr_.data(),
        local_addr_.data_size()) != -1;
    if(ret)
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
      ::close(socket_);
    }
  }

  L_INF("Session initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));
  return ret;
}

//------------------------------------------------------------------------------

auto ClientSession::write(
    SmBufEx::const_iterator buf_begin,
    SmBufEx::const_iterator buf_end,
    const size_t & position) -> ssize_t
{
  if(settings_.opt.request_type() != SrvReq::read)
  {
    throw std::runtime_error(
        "Wrong use method (can't use when request type != read");
  }

  if(file_out_.is_open())
  {
    if(auto buf_size=std::distance(buf_begin, buf_end); buf_size > 0)
    {
      ssize_t begin_pos=file_out_.tellp();
      if(begin_pos != (ssize_t)position)
      {
        L_WRN("Change write position "+std::to_string(file_out_.tellp())+
              " -> "+std::to_string(position));
        file_out_.seekp(position);
      }

      begin_pos=file_out_.tellp();
      if(file_out_.tellp() != (ssize_t)position)
      {
        L_ERR("File stream wrong seek position "+std::to_string(position));
        //set_error_if_first(0, "Server write stream seek failed");
        return -1;
      }

      file_out_.write(& * buf_begin, buf_size);
      ssize_t end_pos = file_out_.tellp();
      if(end_pos < 0)
      {
        L_ERR("File stream wrong write at pos "+std::to_string(position));
        //set_error_if_first(0, "Server write stream failed - no writed data");
        return -1;
      }

      return end_pos - begin_pos;
    }
  }
  else
  {
    L_ERR("File stream not opened");
    //set_error_if_first(0, "Server write stream not opened");
    return -1;
  }

  return 0;
}

//------------------------------------------------------------------------------

auto ClientSession::read(
    SmBufEx::iterator buf_begin,
    SmBufEx::iterator buf_end,
    const size_t & position) -> ssize_t
{
  if(settings_.opt.request_type() != SrvReq::write)
  {
    throw std::runtime_error(
        "Wrong use method (can't use when request type != write");
  }

  if(file_in_.is_open())
  {
    auto buf_size = std::distance(buf_begin, buf_end);
    L_DBG("Generate block (buf size "+std::to_string(buf_size)+
          "; position "+std::to_string(position)+")");

    if(ssize_t curr_pos=file_in_.tellg(); curr_pos != (ssize_t)position)
    {
      if(curr_pos >=0)
      {
        L_WRN("Change read position "+std::to_string(curr_pos)+
              " -> "+std::to_string(position));
      }
      file_in_.seekg(position, std::ios_base::beg);
    }

    ssize_t file_size_ = settings_.opt.was_set_tsize() ? settings_.opt.tsize() : 0;

    auto ret_size = static_cast<ssize_t>(file_size_) - (ssize_t)position;
    if(ret_size > 0)
    {

      try
      {
        file_in_.read(& *buf_begin, buf_size);
      }
      catch (const std::system_error & e)
      {
        if(file_in_.fail())
        {
          L_ERR(std::string{"Error: "}+e.what()+" ("+
                std::to_string(e.code().value())+")");
        }
      }

      if(ret_size > buf_size) ret_size = buf_size;
    }
    return ret_size;
  }

  // not opened
  L_ERR("File stream not opened");
  //set_error_if_first(0, "Server read stream not opened");
  return -1;
}

//------------------------------------------------------------------------------

bool ClientSession::active() const
{
  return ((settings_.opt.request_type() == SrvReq::read)  && file_out_ .is_open()) ||
         ((settings_.opt.request_type() == SrvReq::write) && file_in_.is_open());
}

// -----------------------------------------------------------------------------

void ClientSession::close()
{
  if(file_in_ .is_open()) file_in_ .close();
  if(file_out_.is_open()) file_out_.close();

  ::close(socket_);
}

// -----------------------------------------------------------------------------

void ClientSession::cancel()
{
  if(file_out_.is_open())
  {
    file_out_.close();

    if(filesystem::exists(settings_.file_local))
    {
      L_INF("Remove file '"+settings_.file_local+"'");
      filesystem::remove(settings_.file_local);
    }
  }

  close();
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

  if(!active())
  {
    L_ERR("File streams not active");
    cancel();
    stopped_ = true;
    return ClientSessionResult::fail_run;
  }

  // Prepare
  //bool last_blk_processed_{false};
  //uint16_t retr_count{0U};
  SmBufEx local_buf{0xFFFFU};
  time_t oper_time_{0};

  auto timeout_pass = [&]()
      { return (time(nullptr) - oper_time_) < (settings_.opt.timeout() + 1); };

  auto timeout_reset = [&]()
      { oper_time_ = time(nullptr); };


  stage_ = 0U;
  while(!is_finished())
  {
    switch(stat_)
    {
      case State::need_init: // ------------------------------------------------
        if(init_session())
        {
          if(was_error())
          {
            switch_to(State::error_and_stop);
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
          switch_to(State::ack_options);
        }
        else // if error tx
        {
          switch_to(State::error_and_stop);
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
            break;
          case TripleResult::fail:
            switch_to(State::error_and_stop);
            break;
        }

        break;
















      default:
        switch_to(State::finish);
        break;
    }
    //sleep(2); break;
  } // end main loop


  //cancel();
  //stopped_ = true;

  L_INF("Finish session");

  return ClientSessionResult::ok;
}

// -----------------------------------------------------------------------------

auto ClientSession::run(int argc, char * argv[]) -> ClientSessionResult
{
  if(init(argc, argv))
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
  return settings_.opt.blksize();
}

// -----------------------------------------------------------------------------

void ClientSession::construct_request(SmBufEx & buf) const
{
  buf.clear();

  buf.push_data((uint16_t) settings_.opt.request_type());

  buf.push_data(settings_.opt.filename());

  buf.push_data(to_string(settings_.opt.transfer_mode()));

  if(settings_.opt.was_set_blksize())
  {
    std::string name{constants::name_blksize};
    std::string value{std::to_string(settings_.opt.blksize())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }
  if(settings_.opt.was_set_timeout())
  {
    std::string name{constants::name_timeout};
    std::string value{std::to_string(settings_.opt.timeout())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }
  if(settings_.opt.was_set_windowsize())
  {
    std::string name{constants::name_windowsize};
    std::string value{std::to_string(settings_.opt.windowsize())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }
  if(settings_.opt.was_set_tsize())
  {
    std::string name{constants::name_tsize};
    std::string value{std::to_string(settings_.opt.tsize())};
    buf.push_data(name, value);
    L_DBG("Add option "+name+"="+value);
  }

  L_DBG("Construct request '"+settings_.opt.request_type()+
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

  ssize_t ret = read(
      buf.begin() + buf.data_size(),
      buf.begin() + buf.data_size() + block_size(),
      (stage_-1U) *  block_size());


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
        settings_.srv_addr.as_sockaddr_ptr(),
        settings_.srv_addr.data_size());

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

  // Check client address is right
  if(rx_client.eqv_addr_only(settings_.srv_addr))
  {
    L_DBG(rx_msg+" from client");
  }
  else
  {
    L_WRN("Alarm! Intrusion detect from addr "+rx_client.str()+
          " with data: "+rx_msg+". Ignore pkt!");
    return TripleResult::nop;
  }

  ssize_t rx_stage = (ssize_t)stage_ +
                      (ssize_t)rx_blk - (ssize_t)blk_num_local();

  switch(rx_op)
  {
    case 3U: // DATA
      if(stat_ == State::data_rx)
      {
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

        ssize_t stored_data_size =  write(
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
      break;
    case 6U: // OPTION ACK
      if(stat_ == State::ack_options)
      {
        Options confirm_opt;
        confirm_opt.buffer_parse_ack(
            buf,
            rx_pkt_size); // no nested logging
            //std::bind(
            //    & ClientSession::log,
            //    this,
            //    std::placeholders::_1,
            //    std::placeholders::_2));

        { // blksize
          std::string oname = "'" + std::string{constants::name_blksize} + "'";
          if(settings_.opt.was_set_blksize() == confirm_opt.was_set_blksize())
          {
            if(settings_.opt.was_set_blksize())
            {
              L_DBG("Ack option "+oname+"="+std::to_string(confirm_opt.blksize()));
              if(settings_.opt.blksize() != confirm_opt.blksize())
              {
                L_WRN("Try change ack value for option "+oname+"; Ignore new value!");
                // TODO: do change value (?)
              }
            }
          }
          else // desync
          {
            if(settings_.opt.was_set_blksize())
            {
              L_WRN("Option "+oname+" not confirmed! Ignore self value");
              // TODO: reset option
            }
            else
            {
              L_WRN("Option "+oname+" not required but present! Ignore new value");
            }
          }
        }


        if(confirm_opt.was_set_timeout())
        {
          L_DBG("Ack option "+std::string{constants::name_timeout}+
                "="+std::to_string(confirm_opt.timeout()));
          if(settings_.opt.timeout() != confirm_opt.timeout())
          {
            L_WRN("Try change ack value for option "+
                  std::string{constants::name_timeout}+"; Ignore!");
            // TODO: do change value (?)
          }
        }
        if(confirm_opt.was_set_windowsize())
        {
          L_DBG("Ack option "+std::string{constants::name_windowsize}+
                "="+std::to_string(confirm_opt.windowsize()));
          if(settings_.opt.windowsize() != confirm_opt.windowsize())
          {
            L_WRN("Try change ack value for option "+
                  std::string{constants::name_windowsize}+"; Ignore!");
            // TODO: do change value (?)
          }
        }
        if(confirm_opt.was_set_tsize())
        {
          L_DBG("Ack option "+std::string{constants::name_tsize}+
                "="+std::to_string(confirm_opt.tsize()));
          if(settings_.opt.tsize() != confirm_opt.tsize())
          {
            L_WRN("Try change ack value for option "+
                  std::string{constants::name_tsize}+"; Ignore!");
            // TODO: do change value (?)
          }
        }
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
