/**
 * \file tftpSession.cpp
 * \brief TFTP session class
 *
 *  TFTP session class
 *  Before use class need assign settings
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <regex>
#include <unistd.h>

#include "tftpSession.h"
#include "tftpSmBufEx.h"
#include "tftpDataMgrFile.h"

namespace tftp
{

// -----------------------------------------------------------------------------

Session::Session(pSrvSettingsStor new_settings):
    Base(new_settings),
    stat_{State::need_init},
    finished_{false},
    my_addr_{},
    cl_addr_{},
    socket_{0},
    stage_{0U},
    error_code_{0U},
    error_message_{""},
    opt_{},
    file_man_{nullptr}
{
}

// -----------------------------------------------------------------------------

Session::Session():
    Session(nullptr)
{
}

// -----------------------------------------------------------------------------

Session::Session(const Base & base):
    Session(base.get_ptr())
{
}

// -----------------------------------------------------------------------------

Session::~Session()
{
}

// -----------------------------------------------------------------------------

auto Session::operator=(Session && val) -> Session &
{
  if(this != & val)
  {
    settings_ = val.settings_;
    stat_.store(val.stat_);
    finished_.store(val.finished_);
    my_addr_       = val.my_addr_;
    cl_addr_       = val.cl_addr_;
    socket_        = val.socket_;
    stage_         = val.stage_;
    error_code_    = val.error_code_;
    std::swap(error_message_, val.error_message_);
    std::swap(opt_, val.opt_);
    std::swap(file_man_, val.file_man_);
  }

  return *this;
}

// -----------------------------------------------------------------------------

bool Session::switch_to(const State & new_state)
{
  bool ret = (stat_ == new_state);

  if(!ret)
  {
    switch(stat_)
    {
      case State::need_init:
        ret = (new_state == State::finish        ) ||
              (new_state == State::error_and_stop) ||
              (new_state == State::ack_options   ) ||
              (new_state == State::data_tx       ) ||
              (new_state == State::ack_tx        );
        break;
      case State::error_and_stop:
        ret = (new_state == State::finish);
        break;
      case State::ack_options:
        ret = (new_state == State::data_rx) ||
              (new_state == State::data_tx) ||
              (new_state == State::ack_rx ) ||
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
      case State::request: // request not for server
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

bool Session::is_finished() const
{
  return (stat_ == State::finish);
}

// -----------------------------------------------------------------------------

auto Session::block_size() const -> uint16_t
{
  return opt_.blksize();
}

// -----------------------------------------------------------------------------

auto Session::blk_num_local() const -> uint16_t
{
  return (stage_ & 0x000000000000FFFFU);
}

// -----------------------------------------------------------------------------

void Session::socket_close()
{
  close(socket_);
}

// -----------------------------------------------------------------------------

bool Session::prepare(
    const Addr & remote_addr,
    const SmBuf  & pkt_data,
    const size_t & pkt_data_size)
{
  L_INF("Session prepare started");

  bool ret=true;

  my_addr_ = server_addr();
  my_addr_.set_port(0);

  // Init client remote addr
  cl_addr_ = remote_addr;

  // Parse request pkt buffer
  ret = ret && opt_.buffer_parse(
      pkt_data,
      pkt_data_size,
      std::bind(
          & Base::log,
          this,
          std::placeholders::_1,
          std::placeholders::_2));


  L_INF("Session prepare is "+(ret ? "SUCCESSFUL" : "FAIL"));
  return ret;
}

// -----------------------------------------------------------------------------

bool Session::init()
{
  L_INF("Session initialize started");

  // Socket open
  socket_ = socket(my_addr_.family(), SOCK_DGRAM, 0);
  bool ret;
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

  // Bind socket
  if(ret)
  {
    begin_shared();
    ret = bind(
        socket_,
        (struct sockaddr *) my_addr_.data(),
        my_addr_.data_size()) != -1;
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
      close(socket_);
    }
  }

  // Data manager init
  if(ret)
  {
    // Try 1 - DB
    // TODO:: init() for DataMgrDB
    bool init_stream = false; // remove it!

    // Try 2 - File
    if(!init_stream)
    {
      file_man_.release();
      file_man_ = std::make_unique<DataMgrFile>();

      init_stream = file_man_->init(
          *this,
          std::bind(
              & Session::set_error_if_first,
              this,
              std::placeholders::_1,
              std::placeholders::_2),
          opt_);
    }

    if(!init_stream)
    {
      // Ignore if error was set from DataMgr
      set_error_if_first(0, "Unknown stream initialize error; break session");
    }
  }

  L_INF("Session initialise is "+(ret ? (was_error() ? "WAS ERROR":"SUCCESSFUL") : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

void Session::construct_opt_reply(SmBufEx & buf)
{
  buf.clear();

  buf.push_data((uint16_t) 6U);

  if(opt_.was_set_blksize())
  {
    buf.push_data(constants::name_blksize);
    buf.push_data(std::to_string(opt_.blksize()));
  }

  if(opt_.was_set_timeout())
  {
    buf.push_data(constants::name_timeout);
    buf.push_data(std::to_string(opt_.timeout()));
  }

  if(opt_.was_set_tsize())
  {
    buf.push_data(constants::name_tsize);
    buf.push_data(std::to_string(opt_.tsize()));
  }
  if(opt_.was_set_windowsize())
  {
    buf.push_data(constants::name_windowsize);
    buf.push_data(std::to_string(opt_.windowsize()));
  }

  if(buf.data_size() < 4)
  { // Nothing to do
    buf.clear();
  }
  else
  {
    L_DBG("Construct confirm options pkt "+
            std::to_string(buf.data_size())+" octets");
  }
}

// -----------------------------------------------------------------------------

void  Session::construct_error(SmBufEx & buf)
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

void Session::construct_data(SmBufEx & buf)
{
  buf.clear();

  buf.push_data((uint16_t) 3U, blk_num_local());

  ssize_t ret = file_man_->read(
      buf.begin() + buf.data_size(),
      buf.begin() + buf.data_size() + block_size(),
      (stage_-1U) * block_size());


  if(ret >=0)
  {
    L_DBG("Construct data pkt block "+std::to_string(stage_)+
            "; data size "+std::to_string(ret)+" bytes");
    buf.data_size_reset(buf.data_size() + ret);
  }
  else // error prepare data
  {
    L_ERR("Error prepare data");
    set_error_if_first(0, "Failed prepare data to send");
  }
}

// -----------------------------------------------------------------------------

void Session::construct_ack(SmBufEx & buf)
{
  buf.clear();

  buf.push_data((uint16_t)4U, blk_num_local()); // tftp header

  L_DBG("Construct ACK pkt block "+std::to_string(blk_num_local()));
}

// -----------------------------------------------------------------------------
void Session::run()
{
  L_INF("Running session");

  // Prepare
  bool last_blk_processed_{false};
  uint16_t retr_count{0U};
  SmBufEx local_buf{0xFFFFU};
  time_t oper_time_{0};

  auto timeout_pass = [&]()
      { return (time(nullptr) - oper_time_) < (opt_.timeout() + 1); };

  auto timeout_reset = [&]()
      { oper_time_ = time(nullptr); };

  // Main loop
  stage_ = 0U;
  while(!is_finished())
  {
    switch(stat_)
    {
      case State::need_init: // ------------------------------------------------
        if(init())
        {
          if(was_error())
          {
            switch_to(State::error_and_stop);
          }
          else
          {
            switch_to(State::ack_options);
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

      case State::ack_options: // ----------------------------------------------
        if(opt_.was_set_any())
        {
          construct_opt_reply(local_buf);
          transmit_no_wait(local_buf);
        }
        timeout_reset();
        switch(opt_.request_type())
        {
          case SrvReq::unknown:
            switch_to(State::error_and_stop);
            break;
          case SrvReq::read:
            switch_to(State::data_tx);
            stage_ = 1U;
            break;
          case SrvReq::write:
            if(opt_.was_set_any())
            {
              switch_to(State::data_rx);
              stage_ = 1U;
            }
            else
            {
              stage_ = 0U;
              switch_to(State::ack_tx);
            }
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
            if(is_window_close(stage_))
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
        if(++retr_count > get_retransmit_count())
        {
          L_WRN("Retransmit count exceeded ("+std::to_string(retr_count)+
                "); Break session");
          switch_to(State::error_and_stop);
        }
        else
        {
          //step_back_window(stage_);
          switch(opt_.request_type())
          {
            case SrvReq::unknown:
              switch_to(State::error_and_stop);
              break;
            case SrvReq::read:
              switch_to(State::data_tx);
              break;
            case SrvReq::write:
              switch_to(State::ack_tx);
              break;
          }
          timeout_reset();
        }
        break;

      case State::finish: // ---------------------------------------------------
        break; // never do this

      case State::request: // --------------------------------------------------
        break; // never do this, not for server!
    }
  } // end main loop

  socket_close();
  file_man_->close();

  L_INF("Finish session");
}

// -----------------------------------------------------------------------------

void Session::set_error_if_first(
    const uint16_t e_cod,
    std::string_view e_msg)
{
  L_DBG("Try register error #"+std::to_string(e_cod)+
        " '"+std::string(e_msg)+"'");

  if(!was_error())
  {
    L_DBG("Rememver it");
    error_code_ = e_cod;
    error_message_.assign(e_msg);
  }
}

// -----------------------------------------------------------------------------

bool Session::was_error()
{
  return (error_code_ > 0U) || (error_message_.size() > 0U);
}

// -----------------------------------------------------------------------------

bool Session::transmit_no_wait(const SmBufEx & buf)
{
  bool ret = false;

  if(buf.data_size() > 0U)
  {
    ssize_t tx_result_size = sendto(
        socket_,
        buf.data(),
        buf.data_size(),
        0,
        cl_addr_.as_sockaddr_ptr(),
        cl_addr_.data_size());

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

auto Session::receive_no_wait(SmBufEx & buf) -> TripleResult
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
    default:
      rx_msg.append(": FAKE tftp packet");
      break;
  }

  // Check client address is right
  if(rx_client == cl_addr_)
  {
    L_DBG(rx_msg+" from client");
  }
  else
  {
    L_WRN("Alarm! Intrusion detect from addr "+cl_addr_.str()+
          " with data: "+rx_msg+". Ignore pkt!");
    return TripleResult::nop;
  }

  ssize_t rx_stage = (ssize_t)stage_ +
                      (ssize_t)rx_blk - (ssize_t)blk_num_local();

  // Parse packet if need and do receive DATA
  if((rx_op == 3U) && (stat_ == State::data_rx)) // DATA
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

  // Parse packet if need and do receive ACK
  if((rx_op == 4U) && (stat_ == State::ack_rx)) // ACK
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

  return TripleResult::nop;
}

// -----------------------------------------------------------------------------

bool Session::is_window_close(const size_t & curr_stage) const
{
  return (curr_stage % windowsize()) == 0U;
}

// -----------------------------------------------------------------------------

void Session::step_back_window(size_t & curr_stage)
{
  size_t new_stage = curr_stage;
  if(curr_stage > 0U)
  {
    if(windowsize() > 1U)
    {
      new_stage -= curr_stage % windowsize();
    }
    else
    {
      --new_stage;
    }

    if(!new_stage) new_stage = 1U;
  }

  curr_stage = new_stage;
}

// -----------------------------------------------------------------------------

auto Session::windowsize() const -> size_t
{
  if(opt_.windowsize() < 1) return 1U;
                       else return (size_t)opt_.windowsize();
}

// -----------------------------------------------------------------------------

} // namespace tftp

