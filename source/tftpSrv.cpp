/**
 * \file tftp_server.cpp
 * \brief TFTP server class module
 *
 *  TFTP server master class module
 *
 *  License GPL-3.0
 *
 *  \date 13-sep-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2.1
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include "tftpSrv.h"
#include "tftpCommon.h"
#include "tftpSmBuf.h"
#include "tftpAddr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

Srv::Srv():
    Base(),
    sessions_{},
    socket_{-1},
    stop_{false}
{
}

// -----------------------------------------------------------------------------

Srv::~Srv()
{
}

// -----------------------------------------------------------------------------

bool Srv::socket_open()
{
  // Open socket
  {
    auto lk = begin_shared();

    socket_ = socket(settings_->local_base_.family(),
                     SOCK_DGRAM,
                     0);
  }
  if(socket_< 0)
  {
    Buf err_msg_buf(1024, 0);

    L_ERR("socket() error: "+
            std::string{strerror_r(errno,
                                   err_msg_buf.data(),
                                   err_msg_buf.size())});
    return false;
  };

  // Bind
  int bind_result;
  {
    auto lk = begin_shared();

    bind_result = bind(socket_,
                       settings_->local_base_.as_sockaddr_ptr(),
                       settings_->local_base_.data_size());
  }
  if(bind_result != 0)
  {
    Buf err_msg_buf(1024, 0);
    L_ERR("bind() error: "+
           std::string{strerror_r(errno,
                                  err_msg_buf.data(),
                                  err_msg_buf.size())});
    socket_close();
    return false;
  };

  return true;
}

// -----------------------------------------------------------------------------

void Srv::socket_close()
{
  close(socket_);
  socket_ = -1;
}

// -----------------------------------------------------------------------------

bool Srv::init()
{
  L_INF("Server initialise started");

  if(socket_ >= 0) socket_close();

  bool ret = socket_open();

  if(ret) L_INF("Server listening "+get_local_base_str());

  L_INF("Server initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

void Srv::stop()
{
  stop_ = true;
}

// -----------------------------------------------------------------------------
void Srv::main_loop()
{
  // Try init if need
  if(socket_ == 0)
  {
    if(!init()) return;
  }

  // prepare loop
  stop_ = false;
  Addr  client_addr;
  SmBuf pkt_buf(0xFFFFU, 0);

  // do main server loop
  while (!stop_)
  {
    client_addr.data_size() = client_addr.size();

    int bsize = recvfrom(socket_,
                         pkt_buf.data(),
                         pkt_buf.size(),
                         MSG_DONTWAIT,
                         client_addr.as_sockaddr_ptr(),
                         & client_addr.data_size());

    if(bsize >= 9) // minimal size = 2+1+1+4+1
    {
      L_INF("Receive initial pkt (data size "+std::to_string(bsize)+
              " bytes) from "+client_addr.str());

      tftp::Session sss(*this);

      bool ret = sss.prepare(
          client_addr,
          pkt_buf,
          (size_t) bsize);

      if(ret)
      {
        auto new_session = sessions_.emplace(sessions_.end());

        std::get<0>(* new_session) = std::move(sss);

        std::get<1>(* new_session) = std::thread(
            & tftp::Session::run,
            & std::get<0>(* new_session));

      }
    }
    else
    if(bsize > 0)
    {
      L_WRN("Receive fake initial pkt (data size " + std::to_string(bsize) +
            " bytes) from " + client_addr.str());
    }

    // check finished other sessions
    usleep(1000);
    for(auto it = sessions_.begin(); it != sessions_.end(); ++it)
    {
      if(std::get<0>(* it).is_finished())
      {
        std::get<1>(* it).join();
        sessions_.erase(it);
        break;
      }
    }
  }
}

// -----------------------------------------------------------------------------

} // namespace tftp
