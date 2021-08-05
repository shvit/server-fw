/*
 * tftpDataMgrFileRead.cpp
 *
 *  Created on: 3 авг. 2021 г.
 *      Author: svv
 */

#include "tftpDataMgrFileRead.h"

namespace tftp
{

namespace ext
{


//------------------------------------------------------------------------------

DataMgrFileRead::DataMgrFileRead():
    DataMgrFile(),
    file_in_{}
{
}

//------------------------------------------------------------------------------

DataMgrFileRead::~DataMgrFileRead()
{

}

//------------------------------------------------------------------------------

bool DataMgrFileRead::init(
    SrvBase & sett,
    fSetError cb_error,
    const Options & opt)
{
  return false;
}

//------------------------------------------------------------------------------

void DataMgrFileRead::close()
{

}

//------------------------------------------------------------------------------

bool DataMgrFileRead::active() const
{
  return file_in_.is_open();
}

// -----------------------------------------------------------------------------

auto DataMgrFileRead::write(
    SmBufEx::const_iterator buf_begin,
    SmBufEx::const_iterator buf_end,
    const size_t & position) -> ssize_t
{
  throw std::runtime_error(
      "Wrong use method (fail operation 'write' on input stream");
}

// -----------------------------------------------------------------------------

auto DataMgrFileRead::read(
    SmBufEx::iterator buf_begin,
    SmBufEx::iterator buf_end,
    const size_t & position) -> ssize_t
{
  // Check stream opened
  if(!active())
  {
    constexpr std::string_view tmp_msg{"File input stream not active"};
    //L_ERR(tmp_msg);
    //set_error_if_first(0, tmp_msg);
    return -1;
  }

  auto buf_size = std::distance(buf_begin, buf_end);
  L_DBG("Generate block (buf size "+std::to_string(buf_size)+
        "; position "+std::to_string(position)+")");

  // Check and set pisition
  if(ssize_t curr_pos=file_in_.tellg(); curr_pos != (ssize_t)position)
  {
    if(curr_pos >=0)
    {
      L_WRN("Change read position "+std::to_string(curr_pos)+
            " -> "+std::to_string(position));
    }
    file_in_.seekg(position);
  }

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

//------------------------------------------------------------------------------

} // namespace ext

} // namespace tftp

