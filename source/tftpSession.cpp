/**
 * \file tftpSession.cpp
 * \brief TFTP session class
 *
 *  TFTP session class
 *  Before use class need assign settings
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <regex>
#include <netinet/in.h> // sockaddr_in6
#include <unistd.h>

#include "tftpSession.h"

#include <iostream>

namespace tftp
{

// -----------------------------------------------------------------------------

Session::Session(pSettings new_settings):
    Base(new_settings),
    stat_{State::need_init},
    my_addr_{},
    cl_addr_{},
    socket_{0},
    buf_tx_(0xFFFFU, 0),
    buf_rx_(0xFFFFU, 0),
    stage_{0U},
    buf_tx_data_size_{0U},
    oper_time_{0},
    oper_tx_count_{0},
    oper_wait_{false}, // fist stage is TX
    oper_last_block_{0U},
    stop_{false},
    finished_{false},
    manager_{DataMgr{}},
    error_code_{0U},
    error_message_{""},
    last_blk_processed_{false},
    opt_{}
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
    if(settings_.get() != val.settings_.get())
    {
      auto lk = begin_unique(); // write lock

      settings_ = val.settings_;
    }
    stat_.store(val.stat_);
    my_addr_ = val.my_addr_;
    cl_addr_ = val.cl_addr_;
    socket_        = val.socket_;
    std::swap(buf_tx_, val.buf_tx_);
    std::swap(buf_rx_, val.buf_rx_);
    stage_         = val.stage_;
    buf_tx_data_size_   = val.buf_tx_data_size_;
    oper_time_     = val.oper_time_;
    oper_tx_count_ = val.oper_tx_count_;
    oper_wait_     = val.oper_wait_;
    oper_last_block_ = val.oper_last_block_;
    stop_            = val.stop_;
    finished_.store(val.finished_);
    manager_         = std::move(val.manager_);
    error_code_      = val.error_code_;
    std::swap(error_message_, val.error_message_);
    std::swap(last_blk_processed_, val.last_blk_processed_);

    std::swap(opt_, val.opt_);
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
        ret = (new_state == State::finish) ||
              (new_state == State::error_and_stop) ||
              (new_state == State::ack_options) ||
              (new_state == State::data_tx) ||
              (new_state == State::ack_tx) ;
        break;
      case State::error_and_stop:
        ret = (new_state == State::finish);
        break;
      case State::ack_options:
        ret = (new_state == State::data_rx) ||
              (new_state == State::data_tx) ||
              (new_state == State::ack_rx) ||
              (new_state == State::ack_tx);
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
        ret = (new_state == State::data_tx);
              (new_state == State::retransmit);
        break;
      case State::retransmit:
        ret = (new_state == State::data_rx) ||
              (new_state == State::ack_rx);
        break;
      case State::finish: // no way switch to other
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
  //return finished_;
  return (stat_ == State::finish);
}

// -----------------------------------------------------------------------------

uint16_t Session::block_size() const
{
  return opt_.blksize();
}

// -----------------------------------------------------------------------------

bool Session::timeout_pass(const time_t gandicap) const
{
  return (time(nullptr) - oper_time_) < (opt_.timeout() + gandicap);
}

// -----------------------------------------------------------------------------

void Session::timeout_reset()
{
  oper_time_ = time(nullptr);
}

// -----------------------------------------------------------------------------

uint16_t Session::blk_num_local(const uint16_t step) const
{
  return ((stage_ + step) & 0x000000000000FFFFU);
}

// -----------------------------------------------------------------------------

void Session::socket_close()
{
  close(socket_);
  socket_=0;
}

// -----------------------------------------------------------------------------

bool Session::prepare(
    const Addr & remote_addr,
    const SmBuf  & pkt_data,
    const size_t & pkt_data_size)
{
  L_INF("Session prepare started");

  bool ret=true;

  {
    auto lk = begin_shared(); // read lock

    my_addr_ = server_addr();
  }
  my_addr_.set_port(0);

  // Init client remote addr
  cl_addr_ = remote_addr;

  // Parse pkt buffer
  ret = ret && opt_.buffer_parse(
      pkt_data,
      pkt_data_size,
      std::bind(
          & Base::log,
          this,
          std::placeholders::_1,
          std::placeholders::_2));


  // alloc session buffer
  if(ret)
  {
    size_t sess_buf_size_ = (size_t)block_size() + 4U;

    buf_tx_.resize(sess_buf_size_);
    ret = (buf_tx_.size() == sess_buf_size_);
  }


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
    manager_.settings_ = settings_;
    manager_.set_error_ = std::bind(
        & Session::set_error_if_first,
        this,
        std::placeholders::_1,
        std::placeholders::_2);

    ret = manager_.init(opt_) || was_error();
  }

  L_INF("Session initialise is "+(ret ? (was_error() ? "WAS ERROR":"SUCCESSFUL") : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

void Session::construct_opt_reply()
{
  buf_tx_data_size_=0;

  push_data((uint16_t) 6U);

  if(opt_.was_set_blksize())
  {
    push_data(constants::name_blksize);
    push_data(std::to_string(opt_.blksize()));
  }

  if(opt_.was_set_timeout())
  {
    push_data(constants::name_timeout);
    push_data(std::to_string(opt_.timeout()));
  }

  if(opt_.was_set_tsize())
  {
    push_data(constants::name_tsize);
    push_data(std::to_string(opt_.tsize()));
  }
  if(opt_.was_set_windowsize())
  {
    push_data(constants::name_windowsize);
    push_data(std::to_string(opt_.windowsize()));
  }

  if(buf_tx_data_size_ < 4)
  { // Nothing to do
    buf_tx_data_size_ = 0;
  }
  else
  {
    L_DBG("Construct confirm options pkt "+
            std::to_string(buf_tx_data_size_)+" octets");
  }
}

// -----------------------------------------------------------------------------
void Session::construct_error(
    const uint16_t e_code,
    std::string_view e_msg)
{
  buf_tx_data_size_=0;

  push_data((uint16_t) 5U);
  push_data(e_code);
  push_data(e_msg);

  L_DBG("Construct error pkt code "+std::to_string(e_code)+
        " '"+std::string(e_msg)+"'; "+std::to_string(buf_tx_data_size_)+
        " octets");
}

// -----------------------------------------------------------------------------
void  Session::construct_error()
{
  if(error_message_.size()) construct_error(error_code_, error_message_);
                       else construct_error(0, "Undefined error");
}

// -----------------------------------------------------------------------------

auto Session::construct_data() -> ssize_t
{
  buf_tx_data_size_ = 0;

  push_data((uint16_t) 3U);
  push_data(blk_num_local());

  ssize_t ret = manager_.tx(
      buf_tx_.begin() + buf_tx_data_size_,
      buf_tx_.begin() + buf_tx_data_size_ + block_size(),
      (stage_-1U) * block_size());

  if(ret >=0)
  {
    buf_tx_data_size_ += ret;
    L_DBG("Construct data pkt block "+std::to_string(stage_)+
            "; data size "+std::to_string(buf_tx_data_size_-4)+" bytes");
  }
  else // error prepare data
  {
    L_ERR("Error prepare data");
    buf_tx_data_size_ = 0;
    set_error_if_first(0, "Failed prepare data to send");
    //construct_error();
  }
  return ret;
}

// -----------------------------------------------------------------------------

void Session::construct_ack()
{
  buf_tx_data_size_ = 0;

  push_data((uint16_t)4U);
  push_data(blk_num_local());

  L_DBG("Construct ACK pkt block "+std::to_string(blk_num_local()));
}

// -----------------------------------------------------------------------------
void Session::run()
{
  L_INF("Running session");

  // Prepare
  oper_tx_count_   = 0;
  oper_last_block_ = 0;
  stage_           = 0; // processed block number
  buf_tx_data_size_     = 0;

  // Main loop
  while(!is_finished())
  {
    switch(stat_)
    {
      case State::need_init:
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

      case State::error_and_stop:
        if(was_error())
        {
          construct_error();
          transmit_no_wait();
        }
        switch_to(State::finish);
        break;

      case State::ack_options:
        if(opt_.was_set_any())
        {
          construct_opt_reply();
          transmit_no_wait();
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

      case State::data_tx:
        {
          ssize_t data_size = construct_data();
          if(data_size >= 0)
          {
            transmit_no_wait();
            ++stage_;
            last_blk_processed_ = (data_size != (ssize_t)block_size());

            if(is_window_close() || last_blk_processed_)
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

      case State::data_rx:
        switch(receive_no_wait())
        {
          case TripleResult::nop:
            if(!timeout_pass(1))
            {
              switch_to(State::retransmit);
            }
            break;
          case TripleResult::ok:
            if(is_window_close())
            {
              switch_to(State::ack_tx);
            }
            else
            {
              ++stage_;
            }
            timeout_reset();
            break;
          case TripleResult::fail:
            switch_to(State::error_and_stop);
            break;
        }
        break;

      case State::ack_tx:
        construct_ack();
        transmit_no_wait();
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

      case State::ack_rx:
        switch(receive_no_wait())
        {
          case TripleResult::nop:
            if(!timeout_pass(1))
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

      case State::retransmit:
        // TODO: retry send ...
        timeout_reset();
        break;

      case State::finish:
        break; // never do this
    }
  } // end main loop

  socket_close();
  manager_.close();

  //finish_run:
  L_INF("Finish session");
}
// -----------------------------------------------------------------------------

void Session::set_error_if_first(
    const uint16_t e_cod,
    std::string_view e_msg)
{
  if(!was_error())
  {
    error_code_ = e_cod;
    error_message_.assign(e_msg);
  }
}

// -----------------------------------------------------------------------------

bool Session::was_error()
{
  return error_code_ || error_message_.size();
}

// -----------------------------------------------------------------------------

bool Session::is_stage_transmit() const noexcept
{
  return !oper_wait_;
}

// -----------------------------------------------------------------------------

void Session::set_stage_receive() noexcept
{
  oper_wait_ = true;
  L_DBG("OK");
}

// -----------------------------------------------------------------------------

void Session::set_stage_transmit() noexcept
{
  oper_wait_ = false;
  L_DBG("OK");
}

// -----------------------------------------------------------------------------

bool Session::transmit_no_wait()
{
  bool ret = false;

  if(buf_tx_data_size_ > 0U)
  {
    ssize_t tx_result_size = sendto(
        socket_,
        buf_tx_.data(),
        buf_tx_data_size_,
        0,
        cl_addr_.as_sockaddr_ptr(),
        cl_addr_.data_size());

    ret = (tx_result_size == (ssize_t)buf_tx_data_size_);

    if(ret) // Good send
    {
      L_DBG("Success send packet "+std::to_string(buf_tx_data_size_)+
            " octets");
      buf_tx_data_size_ = 0;
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
              " from "+std::to_string(buf_tx_data_size_));
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

auto Session::receive_no_wait() -> TripleResult
{
  Addr rx_client;
  rx_client.data_size() = rx_client.size();

  ssize_t rx_pkt_size = recvfrom(
      socket_,
      buf_rx_.data(),
      buf_rx_.size(),
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

  // Extract meta info
  uint16_t rx_op  = (rx_pkt_size > 3) ? buf_rx_.get_ntoh<uint16_t>(0U) : 0U;
  uint16_t rx_blk = (rx_pkt_size > 3) ? buf_rx_.get_ntoh<uint16_t>(2U) : 0U;
  uint16_t rx_data_size = (rx_pkt_size > 3) ?
      (rx_pkt_size > 0xFFFF ? 0xFFFFU : rx_pkt_size - 4) : 0U;

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
                    " '"+buf_rx_.get_string(4U)+"'");
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

    ssize_t stored_data_size =  manager_.rx(
        buf_rx_.begin() + 2*sizeof(uint16_t),
        buf_rx_.begin() + rx_pkt_size,
        (stage_ - 1) * block_size());
    if(stored_data_size < 0)
    {
      L_ERR("Error from store data manager");
      set_error_if_first(0, "Error when try to store data");
      return  TripleResult::fail;
    }
    last_blk_processed_ = rx_data_size != block_size();
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

bool Session::is_window_close() const
{
  return (stage_ % (size_t)opt_.windowsize()) == 0U;
}


} // namespace tftp

