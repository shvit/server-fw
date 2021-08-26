/**
 * \file tftp_server.cpp
 * \brief TFTP server class module
 *
 *  TFTP server master class module
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/syscall.h>

#include "tftpSrv.h"
#include "tftpCommon.h"
#include "tftpSmBuf.h"
#include "tftpAddr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

Srv::Srv(fLogMsg logger, pSrvSettingsStor sett):
        SrvSettings(sett),
        Logger(logger),
        local_addr_{},
        sessions_{},
        socket_{-1},
        stop_{false},
        stopped_{false}
{
  local_addr_.set_family(AF_INET);
  local_addr_.set_port(constants::default_tftp_port);
}

// -----------------------------------------------------------------------------

Srv::Srv():
    Srv(nullptr, SrvSettingsStor::create())
{
}

// -----------------------------------------------------------------------------

auto Srv::create(fLogMsg logger, pSrvSettingsStor sett) -> pSrv
{
  return std::make_unique<Srv>(logger, sett);
}

// -----------------------------------------------------------------------------

Srv::~Srv()
{
}

// -----------------------------------------------------------------------------

bool Srv::socket_open()
{
  // Open socket
  if((socket_ = socket(local_addr_.family(), SOCK_DGRAM, 0))< 0)
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
                       local_addr_.as_sockaddr_ptr(),
                       local_addr_.data_size());
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

bool Srv::init(std::string_view list_addr)
{
  L_INF("Server initialise started");

  local_addr_.set_string(list_addr);

  if(socket_ >= 0) socket_close();

  bool ret = socket_open();

  if(ret) L_INF("Server listening "+local_addr_.str());

  L_INF("Server initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

void Srv::stop()
{
  stop_.store(true);
}

// -----------------------------------------------------------------------------
void Srv::main_loop()
{
  L_DBG("Runned");

  // prepare loop
  stop_.store(false);
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

      if(auto new_sess = SrvSession::create(*this, *this);
         new_sess->prepare(local_addr_,
                           client_addr,
                           pkt_buf,
                           (size_t) bsize))
      {
        auto bakup_ptr = new_sess.get();

        sessions_.emplace_back(
            RuntimeSrvSession{
                std::move(new_sess),
                std::thread{& SrvSession::run, bakup_ptr}});
      }
    }
    else
    if(bsize > 0)
    {
      L_WRN("Receive fake initial pkt (data size " + std::to_string(bsize) +
            " bytes) from " + client_addr.str());
    }

    // check finished other sessions
    usleep(10000);
    for(auto it = sessions_.begin(); it != sessions_.end(); ++it)
    {
      if(it->first->is_finished())
      {
        it->second.join();
        sessions_.erase(it);
        break;
      }
    }
  }

  stopped_.store(true);
  L_DBG("Stopped");
}

// -----------------------------------------------------------------------------

bool Srv::is_stopped() const
{
  return stopped_;
}

// -----------------------------------------------------------------------------

} // namespace tftp
