/**
 * \file tftp_server.cpp
 * \brief TFTP server class module
 *
 *  TFTP server master class module
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>

#include "tftpSrv.h"
#include "tftpCommon.h"
#include "tftpSmBuf.h"

namespace tftp
{

// -----------------------------------------------------------------------------

Srv::Srv():
    Base(),
    sessions_{},
    socket_{0},
    buffer_(0x10000, 0),
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
  // Get inet family
  int family;
  {
    auto lk = begin_shared();
    family = local_base_as_inet().sin_family;
  }

  // Open socket
  socket_ = socket(family, SOCK_DGRAM, 0);
  if (socket_< 0)
  {
    Buf err_msg_buf(1024, 0);

    L_ERR("socket() error: "+
            std::string{strerror_r(errno,
                                   err_msg_buf.data(),
                                   err_msg_buf.size())});
    return false;
  };

  // Bind
  struct sockaddr * sock_addr;
  socklen_t         sock_size;
  int bind_result;
  {
    auto lk = begin_shared();

    switch(family)
    {
      case AF_INET:
        sock_addr =  (struct sockaddr *) & local_base_as_inet();
        sock_size = sizeof(decltype(local_base_as_inet()));
        break;
      case AF_INET6:
        sock_addr = (struct sockaddr *) & local_base_as_inet6();
        sock_size = sizeof(decltype(local_base_as_inet6()));
        break;
      default:
        L_ERR("Wrong network family id_"+std::to_string(family));
        return false;
    }

    bind_result = bind(socket_, sock_addr, sock_size);
  }

  if(bind_result)
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

void Srv::socket_close()
{
  close(socket_);
  socket_ = 0;
}

// -----------------------------------------------------------------------------

bool Srv::init()
{
  L_INF("Server initialise started");

  bool ret = true;

  if(socket_ > 0) socket_close();

  ret = socket_open();

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
  // prepare
  stop_ = false;
  SmBuf  client_addr(sizeof(struct sockaddr_in6), 0); // max known buffer size
  unsigned int client_addr_size = client_addr.size();

  // main server loop
  while (!stop_)
  {
    int bsize = recvfrom(socket_,
                         buffer_.data(),
                         buffer_.size(),
                         MSG_DONTWAIT,
                         (struct sockaddr *) client_addr.data(),
                         & client_addr_size);

    if(bsize >= 9) // minimal size = 2+1+1+4+1
    {
      L_INF("Receive initial pkt (data size "+std::to_string(bsize)+
              " bytes) from "+
              sockaddr_to_str(client_addr.cbegin(),
                              client_addr.cbegin()+client_addr_size));

      auto new_session = sessions_.emplace(sessions_.end());

      tftp::Session & sss = std::get<0>(* new_session);

      {
        this->begin_shared();
        sss.settings_ = settings_;
      }

      //bool ret =
      sss.prepare(
          client_addr,
          (size_t)client_addr_size,
          buffer_,
          (size_t)bsize);

      //ret = ret && sss.init();

      sss.init();

      //sss.init(client_addr.cbegin(),
      //         client_addr.cbegin() + client_addr_size,
      //         buffer_.cbegin(),
      //         buffer_.cbegin() + bsize);

      std::get<1>(* new_session) = sss.run_thread();
    }
    else
    if(bsize > 0)
    {
      L_WRN("Receive fake initial pkt (data size " + std::to_string(bsize) +
            " bytes) from " +
            sockaddr_to_str(client_addr.cbegin(),
                            client_addr.cbegin() + client_addr_size));
    }

    // check finished
    usleep(1000);
    for(auto it = sessions_.begin(); it != sessions_.end(); ++it)
    {
      if(std::get<0>(* it).finished_)
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
