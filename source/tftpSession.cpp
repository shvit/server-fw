/**
 * \file tftp_session.cpp
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

#include <cassert>
#include <errno.h>
#include <functional>
#include <regex>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr_in6
#include <ctime>
#include <memory>

#include "tftpSession.h"
//#include "error_gen.h"

namespace tftp
{

// ----------------------------------------------------------------------------------

Session::Session():
    Base(),
    request_type_{SrvReq::unknown},
    filename_{""},
    transfer_mode_{TransfMode::unknown},
    client_{},
    socket_{0},
    opt_blksize_{false, 512U},
    opt_timeout_{false, 5U},
    opt_tsize_{false, 0U},
    re_tx_count_{3},
    sess_buffer_tx_(0xFFFFU, 0),
    sess_buffer_rx_(0xFFFFU, 0),
    stage_{0},
    buf_size_tx_{0},
    oper_time_{0},
    oper_tx_count_{0},
    oper_wait_{false}, // fist stage is TX
    oper_last_block_{0},
    stop_{false},
    finished_{false},
    manager_{DataMgr{}},
    error_code_{0},
    error_message_{""}
{
}

// ----------------------------------------------------------------------------------

Session::~Session()
{
}

// ----------------------------------------------------------------------------------

auto Session::get_buf_rx_u16_ntoh(const size_t offset)
{
  //return get_buf_u16_ntoh(sess_buffer_rx_, offset);
  return get_buf_item_ntoh<uint16_t>(sess_buffer_rx_, offset*sizeof(uint16_t));
}

// ----------------------------------------------------------------------------------

void Session::set_buf_tx_u16_hton(const size_t offset, const uint16_t value)
{
  //set_buf_u16_hton(sess_buffer_tx_, offset, value);
  buf_size_tx_ += set_buf_item_hton<uint16_t>(sess_buffer_tx_, offset*sizeof(uint16_t), value);
}

// ----------------------------------------------------------------------------------

uint16_t Session::block_size() const
{
  return std::get<1>(opt_blksize_);
}

// ----------------------------------------------------------------------------------

bool Session::timeout_pass(const time_t gandicap) const
{
  return (time(nullptr) - oper_time_) < (std::get<1>(opt_timeout_) + gandicap);
}

// ----------------------------------------------------------------------------------

void Session::timeout_reset()
{
  oper_time_ = time(nullptr);
}

// ----------------------------------------------------------------------------------

uint16_t Session::blk_num_local(const uint16_t step) const
{
  return ((stage_ + step) & 0x000000000000FFFFU);
}

// ----------------------------------------------------------------------------------

bool Session::socket_open()
{
  LOG(LOG_DEBUG, "Init socket");
  {
    begin_shared();
    socket_ = socket(local_base_as_inet().sin_family, SOCK_DGRAM, 0);
  }
  if (socket_< 0)
  {
    Buf err_msg_buf(1024, 0);
    LOG(LOG_ERR, "socket() error: "+std::string{strerror_r(errno, err_msg_buf.data(), err_msg_buf.size())});
    return false;
  };

  LOG(LOG_DEBUG, "Bind socket");
  int res;
  {
    begin_shared();
    res = bind(socket_, (struct sockaddr *) settings_->local_base_.data(), settings_->local_base_.size());
  }

  if(res)
  {
    Buf err_msg_buf(1024, 0);
    LOG(LOG_ERR, "bind() error: "+std::string{strerror_r(errno, err_msg_buf.data(), err_msg_buf.size())});
    close(socket_);
    return false;
  }

  LOG(LOG_DEBUG, "Socket opened successful");
  return true;
}

// ----------------------------------------------------------------------------------

void Session::socket_close()
{
  close(socket_);
  socket_=0;
}

// ----------------------------------------------------------------------------------

bool Session::init(const Buf::const_iterator addr_begin,
                   const Buf::const_iterator addr_end,
                   const Buf::const_iterator buf_begin,
                   const Buf::const_iterator buf_end)
{
  LOG(LOG_INFO, "Session initialize started");

  client_.assign(addr_begin, addr_end);

  bool ret = client_.size() >= sizeof(struct sockaddr_in); // miminal length

  if(ret)
  {
    local_base_as_inet().sin_port = 0; // with new automatic port number

    request_type_=(SrvReq) be16toh(*((uint16_t *) & *buf_begin));
    LOG(LOG_INFO, "Recognize request type '"+std::string(to_string(request_type_))+"'");

    uint16_t stage = 0;
    auto it_beg = buf_begin + 2U; // 2 Bytes
    std::string opt_key;
    for(auto iter = it_beg; iter != buf_end; ++iter)
    {
      if(*iter == 0)
      {
        switch(++stage)
        {
          case 1: // filename
            if(iter != it_beg) filename_.assign(it_beg, iter);
            if(filename_.size())
            {
              LOG(LOG_INFO, "Recognize filename '"+filename_+"'");
              std::regex regex_filename("^(.*\\/)?(.*)$");
              std::smatch sm_filename;
              if(std::regex_search(filename_, sm_filename, regex_filename) &&
                 (sm_filename.size() == 3) &&
                 (sm_filename[1].str().size()))
              {
                filename_ = sm_filename[2].str();
                LOG(LOG_WARNING, "Strip directory in filename; new filename is '"+filename_+"'");
              }

            }
            break;
          case 2: // mode
            if(iter != it_beg)
            {
              std::string mode{it_beg, iter};

              if(mode == to_string(TransfMode::octet))
              {
                transfer_mode_=TransfMode::octet;
                LOG(LOG_INFO, "Recognize transfer mode "+mode+"'");
              }
              else
              if(mode == to_string(TransfMode::netascii))
              {
                transfer_mode_=TransfMode::netascii;
                LOG(LOG_INFO, "Recognize transfer mode "+mode+"'");
              }
              else
              if(mode == to_string(TransfMode::binary))
              {
                transfer_mode_=TransfMode::octet;
                LOG(LOG_WARNING, "Change mode 'binary' to mode 'octet'");
              }
              //else
              //if(mode == "mail")
              //{
                //transfer_mode_=TransfMode::unknown;
                //LOG(LOG_WARNING, "Unsupported mode 'mail'");
              //}
              else
              {
                LOG(LOG_WARNING, "Unknown transfer mode '"+mode+"'");
              }

            }
            break;
          default: // next options
            if(stage % 2) // for even stage - keys
            {
              if(iter != it_beg) opt_key.assign(it_beg, iter);
                            else opt_key.clear();
            }
            else // for odd stage - values
            {
              if(opt_key.size())
              {
                std::string opt_val{it_beg, iter};

                #define IF_KEY_EQUAL_INT(NAME) \
                    if(opt_key== #NAME)\
                    {\
                      std::tuple_element_t<1, decltype(opt_##NAME##_)> tmp_val=0;\
                      try\
                      {\
                        tmp_val = std::stoi(opt_val);\
                        std::get<0>(opt_##NAME##_) = true;\
                      }\
                      catch (std::invalid_argument) \
                      {\
                        std::get<0>(opt_##NAME##_) = false;\
                        LOG(LOG_WARNING, "Invalid value option "+opt_key+"='"+opt_val+"'");\
                      }\
                      catch (std::out_of_range    )\
                      {\
                        std::get<0>(opt_##NAME##_) = false;\
                        LOG(LOG_WARNING, "Out of range value option "+opt_key+"='"+opt_val+"'");\
                      }\
                      if(std::get<0>(opt_##NAME##_)) std::get<1>(opt_##NAME##_) = tmp_val;\
                    }

                IF_KEY_EQUAL_INT(blksize)
                else
                IF_KEY_EQUAL_INT(timeout)
                else
                IF_KEY_EQUAL_INT(tsize)
                else
                {
                  LOG(LOG_WARNING, "Unknown option '"+opt_key+"'");\
                }

                #undef IF_KEY_EQUAL_INT

              }
              else
              {
                LOG(LOG_WARNING, "Option name not exist, but value present");\
              }
            }
            break;
        } // switch stage
        it_beg = iter + 1;
      }
    } // for loop
  }
  ret = ret && (request_type_ != SrvReq::unknown) && filename_.size() && block_size();

  // alloc session buffer
  if(ret)
  {
    size_t sess_buf_size_ = block_size() + 4;
    if(sess_buf_size_ < 2048) sess_buf_size_= 2048;
    sess_buffer_tx_.assign(sess_buf_size_, 0);
    ret = (sess_buffer_tx_.size() == sess_buf_size_);
  }

  // Socket open
  if(ret)
  {
    ret = socket_open();
  }

  // Data manager init
  if(ret)
  {
    {
      manager_.settings_ = settings_;
      manager_.set_error_ = std::bind(& Session::set_error_if_first, this, std::placeholders::_1, std::placeholders::_2);
    }

    ret = manager_.init(request_type_, filename_);
  }


  LOG(LOG_INFO, "Session initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));
  return ret;
}

// ----------------------------------------------------------------------------------

void Session::check_buffer_tx_size(const size_t size_append)
{
  if((buf_size_tx_+size_append) > sess_buffer_tx_.size())
  {
    sess_buffer_tx_.resize(buf_size_tx_+size_append);
    LOG(LOG_WARNING, "Deprecated buffer resize to "+std::to_string(sess_buffer_tx_.size())+" bytes");
  }
}

// ----------------------------------------------------------------------------------

size_t Session::push_buffer_string(std::string_view str)
{
  Buf::size_type ret_size = set_buf_cont_str(sess_buffer_tx_, buf_size_tx_, str, true);
  buf_size_tx_ += ret_size;
  return ret_size;
}

// ----------------------------------------------------------------------------------

void Session::construct_opt_reply()
{
  buf_size_tx_=0;

  check_buffer_tx_size(2);
  set_buf_tx_u16_hton(0, 6U);

  if(std::get<0>(opt_blksize_))
  {
    LOG(LOG_DEBUG, "Find option 'blksize' for confirm");
    if(size_t temp_size = buf_size_tx_;
       !push_buffer_string("blksize") ||
       !push_buffer_string(std::to_string(std::get<1>(opt_blksize_)))) buf_size_tx_ = temp_size;
  }
  if(std::get<0>(opt_timeout_))
  {
    LOG(LOG_DEBUG, "Find option 'timeout' for confirm");
    if(size_t temp_size = buf_size_tx_;
       !push_buffer_string("timeout") ||
       !push_buffer_string(std::to_string(std::get<1>(opt_timeout_)))) buf_size_tx_ = temp_size;
  }
  if(std::get<0>(opt_tsize_))
  {
    LOG(LOG_DEBUG, "Find option 'tsize' for confirm");
    if(size_t temp_size = buf_size_tx_;
       !push_buffer_string("tsize") ||
       !push_buffer_string(std::to_string(std::get<1>(opt_tsize_)))) buf_size_tx_ = temp_size;
  }

  if(buf_size_tx_ < 4) buf_size_tx_ = 0;

  if(buf_size_tx_) LOG(LOG_DEBUG, "Construct option confirm pkt "+std::to_string(buf_size_tx_)+" octets");
}

// ----------------------------------------------------------------------------------
void Session::construct_error(const uint16_t e_code, std::string_view e_msg)
{
  buf_size_tx_=0;

  check_buffer_tx_size(2);
  set_buf_tx_u16_hton(0, 5);
  set_buf_tx_u16_hton(1, e_code);

  push_buffer_string(e_msg);

  LOG(LOG_DEBUG, "Construct error pkt code "+std::to_string(e_code)+" '"+std::string(e_msg)+"'; "+std::to_string(buf_size_tx_)+" octets");
}

// ----------------------------------------------------------------------------------
void  Session::construct_error()
{
  if(error_message_.size()) construct_error(error_code_, error_message_);
                       else construct_error(0, "Undefined error");
}

// ----------------------------------------------------------------------------------

void Session::construct_data()
{
  buf_size_tx_ = 0;

  set_buf_tx_u16_hton(0, 3);
  set_buf_tx_u16_hton(1, blk_num_local());

  if(ssize_t tx_data_size = manager_.tx(sess_buffer_tx_.begin() + buf_size_tx_,
                                        sess_buffer_tx_.begin() + buf_size_tx_ + block_size(),
                                        (stage_-1) * block_size());
     tx_data_size >= 0)
  {
    buf_size_tx_ += tx_data_size;
    LOG(LOG_DEBUG, "Construct data pkt block "+std::to_string(stage_)+"; data size "+std::to_string(buf_size_tx_-4)+" bytes");
  }
  else // error prepare data
  {
    LOG(LOG_ERR, "Error prepare data");
    buf_size_tx_ = 0;
    set_error_if_first(0, "Failed prepare data to send");
    construct_error();
  }
}

// ----------------------------------------------------------------------------------

void Session::construct_ack()
{
  buf_size_tx_ = 0;
  auto blk = blk_num_local();

  set_buf_tx_u16_hton(0, 4);
  set_buf_tx_u16_hton(1, blk);

  LOG(LOG_DEBUG, "Construct ACK pkt block "+std::to_string(blk));
}

// ----------------------------------------------------------------------------------
void Session::run()
{
  LOG(LOG_INFO, "Running session");

  // checks
  if((request_type_ != SrvReq::read) && (request_type_ != SrvReq::write))
  {
    LOG(LOG_ERR, "Fail request mode ");
    return;
  }

  // Prepare
  timeout_reset();
  oper_tx_count_   = 0;
  set_stage_transmit();
  oper_last_block_ = 0;
  stage_           = 0; // processed block number
  buf_size_tx_     = 0;
  stop_            = false;
  finished_        = false;

  // Main loop
  while(!stop_)
  {

    // 1 tx data if need
    if(!transmit_no_wait()) break;

    // 2 rx data if exist
    if(!receive_no_wait()) break;

    // Check emergency exit
    if(!timeout_pass(2)) // with 2 sec gandicap
    {
      LOG(LOG_WARNING, "Global loop timeout. Break");
      break;
    }

  } // end main loop

  stop_=true; // for exit over break
  socket_close();
  manager_.close();
  LOG(LOG_INFO, "Finish session");

  finished_ = true;
}
// ----------------------------------------------------------------------------------

std::thread Session::run_thread()
{
  std::thread th = std::thread(& tftp::Session::run, this);
  return th;
}

// ----------------------------------------------------------------------------------

void Session::set_error_if_first(const uint16_t e_cod, std::string_view e_msg)
{
  if(!was_error())
  {
    error_code_ = e_cod;
    error_message_.assign(e_msg);
  }
}

// ----------------------------------------------------------------------------------

bool Session::was_error()
{
  return error_code_ || error_message_.size();
}

// ----------------------------------------------------------------------------------

bool Session::is_stage_receive() const noexcept
{
  return oper_wait_;
}

// ----------------------------------------------------------------------------------

bool Session::is_stage_transmit() const noexcept
{
  return !oper_wait_;
}

// ----------------------------------------------------------------------------------

void Session::set_stage_receive() noexcept
{
  oper_wait_ = true;
  LOG(LOG_DEBUG, "OK");
}

// ----------------------------------------------------------------------------------

void Session::set_stage_transmit() noexcept
{
  oper_wait_ = false;
  LOG(LOG_DEBUG, "OK");
}

// ----------------------------------------------------------------------------------

bool Session::transmit_no_wait()
{
  if(!is_stage_transmit()) return true;

  bool ret = true;

  // 1 - prepare data for tx
  if(ret && !buf_size_tx_)
  {
    if(!stage_) construct_opt_reply();

    switch(request_type_)
    {
      case SrvReq::read:
        if(!stage_ && !buf_size_tx_) ++stage_; // if no confirm option then start transmit data
        if(stage_) construct_data();
        if((buf_size_tx_ < (block_size()+4U)))
        {
          oper_last_block_ = stage_;
          LOG(LOG_DEBUG, "Calculated last tx block "+std::to_string(oper_last_block_));
        }
        break;
      case SrvReq::write:
        if(!buf_size_tx_) construct_ack();
        break;
      default:
        ret=false;
        LOG(LOG_ERR, "Wrong request type. Break!");
        break;
    }
  }

  // 2 - Send data
  if(ret && buf_size_tx_)
  {
    if(!oper_tx_count_ || !timeout_pass()) // first try send or time is out
    {
      if(oper_tx_count_ > re_tx_count_)
      {
        LOG(LOG_ERR, "Retransmit count exceeded (" + std::to_string(re_tx_count_) + "). Break!");
        ret=false;
      }

      if(ret) // need send
      {
        auto tx_result_size = sendto(socket_,
                                     sess_buffer_tx_.data(),
                                     buf_size_tx_,
                                     0,
                                     (struct sockaddr *) client_.data(),
                                     client_.size());
        ++oper_tx_count_;
        timeout_reset();
        if(tx_result_size < 0) // Fail TX: error
        {
          LOG(LOG_ERR, "sendto() error");
        }
        else
        if(tx_result_size<(ssize_t)buf_size_tx_) // Fail TX: send lost
        {
          LOG(LOG_ERR, "sendto() lost data error: sended "+std::to_string(tx_result_size)+" from "+std::to_string(buf_size_tx_));
        }
        else // Good TX
        {
          LOG(LOG_DEBUG, "Success send packet "+std::to_string(buf_size_tx_)+" octets");
          buf_size_tx_ = 0;
          set_stage_receive();

          if((request_type_ == SrvReq::write) && oper_last_block_ && (oper_last_block_==stage_))  ret=false; // GOOD EXIT
        }
      }
    }
  }

  if(was_error()) ret=false; // ERROR EXIT

  return ret;
}

// ----------------------------------------------------------------------------------

bool Session::receive_no_wait()
{
  bool ret = true;
  std::string rx_msg;

  decltype(client_) rx_client_sa(sizeof(struct sockaddr_in6), 0);
  socklen_t  rx_client_size = rx_client_sa.size();

  ssize_t rx_result_size = recvfrom(socket_,
                             sess_buffer_rx_.data(),
                             sess_buffer_rx_.size(),
                             MSG_DONTWAIT,
                             (struct sockaddr *) rx_client_sa.data(),
                             & rx_client_size);
  if(rx_result_size < 0)
  {
    switch(errno)
    {
      case EAGAIN:  // OK
        break;
      default:
        ret=false;
        LOG(LOG_ERR, "Error #"+std::to_string(errno)+" when call recvfrom(). Break loop!");;
        break;
    }
  }
  else
  if(rx_result_size > 0)
  {
    rx_msg.assign("Receive pkt ").append(std::to_string(rx_result_size)).append(" octets");

    if(rx_result_size > 3) // minimal tftp pkt size 4 byte
    {
      switch(get_buf_rx_u16_ntoh(0))
      {
        case 4U: // ACK ------------------------------------------------------------------------------------------
          rx_msg.append(": ACK blk ").append(std::to_string(get_buf_rx_u16_ntoh(1)));
          if((request_type_ == SrvReq::read) &&
             (get_buf_rx_u16_ntoh(1) == blk_num_local()))
          {
            LOG(LOG_DEBUG, "OK! "+rx_msg);
            if(oper_last_block_) ret=false; // GOOD EXIT at end
            ++stage_;
            oper_tx_count_ = 0;
            timeout_reset();
            set_stage_transmit();
          }
          else
          {
            LOG(LOG_WARNING, "WRONG! "+rx_msg);
          }
          break;
        case 3U: // DATA ------------------------------------------------------------------------------------------
          rx_msg.append(": DATA blk ").append(std::to_string(get_buf_rx_u16_ntoh(1)));
          rx_msg.append("; data size ").append(std::to_string(rx_result_size));
          if(bool is_next = (get_buf_rx_u16_ntoh(1) == blk_num_local(1U));
              (request_type_ == SrvReq::write) &&
              ((get_buf_rx_u16_ntoh(1) == blk_num_local(0U)) || // current blk
                is_next))  // or next blk
          {
            LOG(LOG_DEBUG, "OK! "+rx_msg);
            if(is_next) ++stage_;

            if(stage_)
            {
              ssize_t stored_data_size =  manager_.rx(sess_buffer_rx_.begin() + 2*sizeof(uint16_t),
                                                      sess_buffer_rx_.begin() + rx_result_size,
                                                      (stage_ - 1) * block_size());
              if(stored_data_size < 0)
              {
                LOG(LOG_ERR, "Error stored data");
                set_error_if_first(0, "Error stored data");
                construct_error();
              }
            }

            if(rx_result_size < (block_size() + 4))
            {
              oper_last_block_ = stage_;
              LOG(LOG_DEBUG, "Calculated last rx block "+std::to_string(oper_last_block_));
            }
            set_stage_transmit();
            oper_tx_count_ = 0;
          }
          else
          {
            LOG(LOG_WARNING, "WRONG! "+rx_msg);
          }
          break;
        case 5U: // ERROR ------------------------------------------------------------------------------------------
          LOG(LOG_ERR, ": ERROR #"+std::to_string(get_buf_rx_u16_ntoh(1)) +
                       " '"+std::string(sess_buffer_rx_.cbegin()+2*sizeof(uint16_t), sess_buffer_rx_.cend())+"'"+
                       rx_msg);
          ret=false;
          break;
      } // ------------------------------------------------------------------------------------------
    }
    else
    {
      LOG(LOG_WARNING, "Packet size too small. Skip!");
    }
  }

  if(ret && !timeout_pass())
  {
    LOG(LOG_DEBUG, "Time is out (oper_tx_count_="+std::to_string(oper_tx_count_)+")");
    set_stage_transmit();
    oper_last_block_=0;
  }

  return ret;
}

// ----------------------------------------------------------------------------------

} // namespace tftp

