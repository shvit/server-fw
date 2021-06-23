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
        pstream_{stream},
        settings_{},
        local_addr_{},
        socket_{0},
        stage_{0U},
        file_in_{},
        file_out_{},
        file_size_{0U}
{
  if(pstream_) settings_.out_header(*pstream_);
}

//------------------------------------------------------------------------------

ClientSession::ClientSession():
    ClientSession(nullptr)
{
}

//------------------------------------------------------------------------------

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

auto ClientSession::run_session() -> ClientSessionResult
{
  if(!active())
  {
    L_ERR("File streams not active");
    cancel();
    return ClientSessionResult::fail_run;
  }

  // TODO: Do tftp protocol
  sleep(2);
  cancel();
  return ClientSessionResult::fail_run;
}

// -----------------------------------------------------------------------------

auto ClientSession::run(int argc, char * argv[]) -> ClientSessionResult
{
  if(!init(argc, argv))
  {
    L_ERR("Wrong run - fail init session");
    cancel();
    return ClientSessionResult::fail_init;
  }

  return run_session();
}

// -----------------------------------------------------------------------------


} // namespace tftp
