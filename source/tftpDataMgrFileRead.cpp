/*
 * tftpDataMgrFileRead.cpp
 *
 *  Created on: 3 авг. 2021 г.
 *      Author: svv
 */

#include "tftpDataMgrFileRead.h"

namespace tftp
{

//------------------------------------------------------------------------------

auto DataMgrFileRead::create(
    fLogMsg logger,
    fSetError err_setter,
    std::string_view filename,
    std::string root_dir) -> pDataMgrFileRead
{
  class Enabler : public DataMgrFileRead
  {
  public:
    Enabler(
        fLogMsg logger,
        fSetError err_setter,
        std::string_view filename,
        std::string root_dir):
            DataMgrFileRead(logger, err_setter, filename, root_dir) {};
  };

  return std::make_unique<Enabler>(logger, err_setter, filename, root_dir);
}

//------------------------------------------------------------------------------

DataMgrFileRead::DataMgrFileRead(
    fLogMsg logger,
    fSetError err_setter,
    std::string_view filename,
    std::string root_dir):
        DataMgrFile(
            logger,
            err_setter,
            filename,
            {root_dir}),
        fs_{}
{
  DataMgrFile::full_search(filename);
}

//------------------------------------------------------------------------------

DataMgrFileRead::~DataMgrFileRead()
{
}

//------------------------------------------------------------------------------

bool DataMgrFileRead::init()
{
  bool ret = filesystem::exists(filename_);

  if(ret) // File exist (OK)
  {
    auto backup_val = fs_.exceptions();
    fs_.exceptions(std::ios::failbit);
    try
    {
      fs_.open(filename_, std::ios_base::in | std::ios::binary);
    }
    catch (const std::system_error & e)
    {
      ret = false;
      L_ERR(std::string{"Error: "}+e.what()+" ("+
            std::to_string(e.code().value())+")");
      set_error_if_first(0U, e.what());
    }
    fs_.close();
    fs_.exceptions(backup_val);
    fs_.open(filename_, std::ios_base::in | std::ios::binary);

    // ... Other
    if(ret)
    {
      file_size_ = filesystem::file_size(filename_);
    }
  }
  else // File not exist (WRONG)
  {
    L_ERR("File not found '" + filename_.string() + "'");
    set_error_if_first(1U, "File not found");
  }

  L_INF("Data manager initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));

  return ret;
}

//------------------------------------------------------------------------------

void DataMgrFileRead::close()
{
  if(active()) fs_ .close();
}

//------------------------------------------------------------------------------

void DataMgrFileRead::cancel()
{
  if(active()) fs_ .close();
}

//------------------------------------------------------------------------------

bool DataMgrFileRead::active() const
{
  return fs_.is_open();
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
    const std::string tmp_msg{"File input stream not active"};
    L_ERR(tmp_msg);
    set_error_if_first(0, tmp_msg);
    return -1;
  }

  auto buf_size = std::distance(buf_begin, buf_end);
  L_DBG("Generate block (buf size "+std::to_string(buf_size)+
        "; position "+std::to_string(position)+")");

  // Check and set pisition
  if(ssize_t curr_pos=fs_.tellg(); curr_pos != (ssize_t)position)
  {
    if(curr_pos >=0)
    {
      L_WRN("Change read position "+std::to_string(curr_pos)+
            " -> "+std::to_string(position));
    }
    fs_.seekg(position);
  }

  auto ret_size = static_cast<ssize_t>(file_size_) - (ssize_t)position;
  if(ret_size > 0)
  {
    try
    {
      fs_.read(& *buf_begin, buf_size);
    }
    catch (const std::system_error & e)
    {
      if(fs_.fail())
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

} // namespace tftp

