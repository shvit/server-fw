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

    std::swap(opt_, val.opt_);
  }

  return *this;
}

// -----------------------------------------------------------------------------

bool Session::is_finished() const
{
  return finished_;
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

void Session::construct_data()
{
  buf_tx_data_size_ = 0;

  push_data((uint16_t) 3U);
  push_data(blk_num_local());

  if(ssize_t tx_data_size = manager_.tx(
         buf_tx_.begin() + buf_tx_data_size_,
         buf_tx_.begin() + buf_tx_data_size_ + block_size(),
         (stage_-1U) * block_size());
     tx_data_size >= 0)
  {
    buf_tx_data_size_ += tx_data_size;
    L_DBG("Construct data pkt block "+std::to_string(stage_)+
            "; data size "+std::to_string(buf_tx_data_size_-4)+" bytes");
  }
  else // error prepare data
  {
    L_ERR("Error prepare data");
    buf_tx_data_size_ = 0;
    set_error_if_first(0, "Failed prepare data to send");
    construct_error();
  }
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

  // checks
  if((opt_.request_type() != SrvReq::read) &&
     (opt_.request_type() != SrvReq::write))
  {
    L_ERR("Fail request mode");
    finished_ = true;
    return;
  }

  // Prepare
  timeout_reset();
  oper_tx_count_   = 0;
  set_stage_transmit();
  oper_last_block_ = 0;
  stage_           = 0; // processed block number
  buf_tx_data_size_     = 0;
  stop_            = false;
  finished_        = false;

  // Main loop
  while(!stop_)
  {

    // 0 Processing errors (if was)
    if(was_error())
    {
      construct_error();
      stop_ = true;
    }

    // 1 tx data if need
    if(!transmit_no_wait()) break;

    // 2 rx data if exist
    if(!receive_no_wait()) break;

    // Check emergency exit
    if(!timeout_pass(2)) // with 2 sec gandicap
    {
      L_WRN("Global loop timeout. Break");
      break;
    }

  } // end main loop

  stop_=true; // for exit over break
  socket_close();
  manager_.close();
  L_INF("Finish session");

  finished_ = true;
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
  if(!is_stage_transmit()) return true;

  bool ret = true;

  // 1 - prepare data for tx
  if(ret && !buf_tx_data_size_)
  {
    if(!stage_) construct_opt_reply();

    switch(opt_.request_type())
    {
      case SrvReq::read:
        if(!stage_ && !buf_tx_data_size_) ++stage_;// if no conf opt -> start tx data
        if(stage_) construct_data();
        if((buf_tx_data_size_ < (block_size()+4U)))
        {
          oper_last_block_ = stage_;
          L_DBG("Calculated last tx block "+std::to_string(oper_last_block_));
        }
        break;
      case SrvReq::write:
        if(!buf_tx_data_size_) construct_ack();
        break;
      default:
        ret=false;
        L_ERR("Wrong request type. Break!");
        break;
    }
  }

  // 2 - Send data
  if(ret && buf_tx_data_size_)
  {
    if(!oper_tx_count_ || !timeout_pass()) // first try send or time is out
    {
      if(oper_tx_count_ > get_retransmit_count())
      {
        L_ERR("Retransmit count exceeded ("+std::to_string(get_retransmit_count())+
                "). Break!");
        ret=false;
      }

      if(ret) // need send
      {
        auto tx_result_size = sendto(socket_,
                                     buf_tx_.data(),
                                     buf_tx_data_size_,
                                     0,
                                     cl_addr_.as_sockaddr_ptr(),
                                     cl_addr_.size());
        ++oper_tx_count_;
        timeout_reset();
        if(tx_result_size < 0) // Fail TX: error
        {
          L_ERR("sendto() error");
        }
        else
        if(tx_result_size<(ssize_t)buf_tx_data_size_) // Fail TX: send lost
        {
          L_ERR("sendto() lost data error: sended "+
                  std::to_string(tx_result_size)+
                  " from "+std::to_string(buf_tx_data_size_));
        }
        else // Good TX
        {
          L_DBG("Success send packet "+std::to_string(buf_tx_data_size_)+
                  " octets");
          buf_tx_data_size_ = 0;
          set_stage_receive();

          if((opt_.request_type() == SrvReq::write) &&
             oper_last_block_ &&
             (oper_last_block_==stage_))
          {
            ret = false; // GOOD EXIT
          }
        }
      }
    }
  }

  if(was_error()) ret = false; // ERROR EXIT

  return ret;
}

// -----------------------------------------------------------------------------

bool Session::receive_no_wait()
{
  bool ret = true;
  std::string rx_msg;

  SmBuf rx_client_sa(sizeof(struct sockaddr_in6), 0);
  socklen_t  rx_client_size = rx_client_sa.size();

  ssize_t rx_data_size = recvfrom(
      socket_,
      buf_rx_.data(),
      buf_rx_.size(),
      MSG_DONTWAIT,
      (struct sockaddr *) rx_client_sa.data(),
      & rx_client_size);

  if(rx_data_size < 0)
  {
    switch(errno)
    {
      case EAGAIN:  // OK
        break;
      default:
        ret=false;
        L_ERR("Error #"+std::to_string(errno)+
                " when call recvfrom(). Break loop!");
        break;
    }
  }
  else
  if(rx_data_size > 0)
  {
    rx_msg.assign("Receive pkt ").
           append(std::to_string(rx_data_size)).
           append(" octets");

    if(rx_data_size > 3) // minimal tftp pkt size 4 byte
    {
      uint16_t rx_op  = buf_rx_.get_ntoh<uint16_t>(0U);
      uint16_t rx_blk = buf_rx_.get_ntoh<uint16_t>(2U);

      switch(rx_op)
      {
        case 4U: // ACK --------------------------------------------------------
          rx_msg.append(": ACK blk ").append(std::to_string(rx_blk));

          if((opt_.request_type() == SrvReq::read) &&
             (rx_blk == blk_num_local()))
          {
            L_DBG("OK! "+rx_msg);
            if(oper_last_block_) ret=false; // GOOD EXIT at end
            ++stage_;
            oper_tx_count_ = 0;
            timeout_reset();
            set_stage_transmit();
          }
          else
          {
            L_WRN("WRONG! "+rx_msg);
          }
          break;
        case 3U: // DATA -------------------------------------------------------
          rx_msg.append(": DATA blk ").append(std::to_string(rx_blk)).
                 append("; data size ").append(std::to_string(rx_data_size));

          if(bool is_next = (rx_blk == blk_num_local(1U));
              (opt_.request_type() == SrvReq::write) &&
              ((rx_blk == blk_num_local(0U)) || // current blk
                is_next))  // or next blk
          {
            L_DBG("OK! "+rx_msg);
            if(is_next) ++stage_;

            if(stage_)
            {
              ssize_t stored_data_size =  manager_.rx(
                  buf_rx_.begin() + 2*sizeof(uint16_t),
                  buf_rx_.begin() + rx_data_size,
                  (stage_ - 1) * block_size());
              if(stored_data_size < 0)
              {
                L_ERR("Error stored data");
                set_error_if_first(0, "Error stored data");
                construct_error();
              }
            }

            if(rx_data_size < (block_size() + 4))
            {
              oper_last_block_ = stage_;
              L_DBG("Calculated last rx block "+
                      std::to_string(oper_last_block_));
            }
            set_stage_transmit();
            oper_tx_count_ = 0;
          }
          else
          {
            L_WRN("WRONG! "+rx_msg);
          }
          break;
        case 5U: // ERROR ------------------------------------------------------
          L_ERR(": ERROR #"+std::to_string(rx_blk) + " '"+
                std::string(buf_rx_.cbegin()+2*sizeof(uint16_t),
                            buf_rx_.cend())+"'"+
                rx_msg);
          ret=false;
          break;
      } // ---------------------------------------------------------------------
    }
    else
    {
      L_WRN("Packet size too small. Skip!");
    }
  }

  if(ret && !timeout_pass())
  {
    L_DBG("Time is out (oper_tx_count_="+
            std::to_string(oper_tx_count_)+")");
    set_stage_transmit();
    oper_last_block_=0;
  }

  return ret;
}

// -----------------------------------------------------------------------------

} // namespace tftp

