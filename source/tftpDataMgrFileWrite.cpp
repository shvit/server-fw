/**
 * \file tftpDataMgrFileWrite.cpp
 * \brief DataMgrFileWrite class module
 *
 *  License GPL-3.0
 *
 *  \date 03-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "tftpDataMgrFileWrite.h"

namespace tftp
{

//------------------------------------------------------------------------------

auto DataMgrFileWrite::create(
    fLogMsg logger,
    fSetError err_setter,
    std::string_view filename,
    std::string root_dir) -> pDataMgrFileWrite
{
  class Enabler : public DataMgrFileWrite
  {
  public:
    Enabler(
        fLogMsg logger,
        fSetError err_setter,
        std::string_view filename,
        std::string root_dir):
          DataMgrFileWrite(logger, err_setter, filename, root_dir) {};
  };

  return std::make_unique<Enabler>(logger, err_setter, filename, root_dir);
}

//------------------------------------------------------------------------------

DataMgrFileWrite::DataMgrFileWrite(
    fLogMsg logger,
    fSetError err_setter,
    std::string_view filename,
    std::string root_dir):
        DataMgrFile(
            logger,
            err_setter),
        fs_{},
        attr_{}
{
  filename_ = root_dir;
  filename_ /= filename;
}

//------------------------------------------------------------------------------

DataMgrFileWrite::~DataMgrFileWrite()
{
}

//------------------------------------------------------------------------------

bool DataMgrFileWrite::init()
{
  bool ret = !filesystem::exists(filename_);

  if(ret) // File not exist (OK)
  {
    fs_.exceptions(fs_.exceptions() | std::ios::failbit);
    try
    {
      fs_.open(filename_, std::ios_base::out | std::ios::binary);
      fs_.write(nullptr, 0U);
    }
    catch (const std::system_error & e)
    {
      ret = false;
      L_ERR(std::string{"Error: "}+e.what()+" ("+
            std::to_string(e.code().value())+")");
      set_error_if_first(0U, e.what());
    }
  }
  else // File exist (WRONG)
  {
    L_ERR("File already exists '"+filename_.string()+"'");
    set_error_if_first(6U, "File already exists");
  }

  L_INF("Data manager initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));

  return ret;
}

//------------------------------------------------------------------------------

void DataMgrFileWrite::close()
{
  if(active())
  {
    fs_ .close();

    // CHOWN
    Buf err_msg_buf(1024, 0);

    auto & user = attr_.own_user();
    auto &  grp  = attr_.own_grp();
    if((user.size() > 0U) ||
       (grp.size() > 0U))
    {
      L_DBG("Try set chown '"+user+"':'"+grp+"'");

      auto ret = chown(
          filename_.c_str(),
          get_uid_by_name(user),
          get_gid_by_name(grp));

      if(ret < 0)
      {
        L_WRN("Wrong chown operation file '"+filename_.string()+"': "+
              std::string{strerror_r(errno,
                                     err_msg_buf.data(),
                                     err_msg_buf.size())});
      }
    }

    // CHMOD
    auto & mode = attr_.mode();
    Perms curr_perm = Perms::none;
    std::string perm_str{"-"};
    if(mode & S_IRUSR) { curr_perm |= Perms::owner_read; perm_str.append("r"); } else perm_str.append("-");
    if(mode & S_IWUSR) { curr_perm |= Perms::owner_write; perm_str.append("w"); } else perm_str.append("-");
    perm_str.append("-");
    if(mode & S_IRGRP) { curr_perm |= Perms::group_read; perm_str.append("r"); } else perm_str.append("-");
    if(mode & S_IWGRP) { curr_perm |= Perms::group_write; perm_str.append("w"); } else perm_str.append("-");
    perm_str.append("-");
    if(mode & S_IROTH) { curr_perm |= Perms::others_read; perm_str.append("r"); } else perm_str.append("-");
    if(mode & S_IWOTH) { curr_perm |= Perms::others_write; perm_str.append("w"); } else perm_str.append("-");
    L_DBG("Try set chmod as '"+perm_str+"'");

    std::error_code e;
    permissions(filename_, curr_perm, e);
    if(e.value())
    {
      L_WRN("Wrong chmod operation: "+e.message());
    }
  }
}

//------------------------------------------------------------------------------

void DataMgrFileWrite::cancel()
{
  if(active())
  {
    fs_ .close();

    // Delete file
    if(filesystem::exists(filename_) &&
       filesystem::is_regular_file(filename_))
    {
      std::error_code e;
      if(!filesystem::remove(filename_, e))
      {
        L_WRN("Error delete file '"+filename_.string()+"': "+e.message());
      }
    }
  }
}

//------------------------------------------------------------------------------

bool DataMgrFileWrite::active() const
{
  return fs_.is_open();
}

// -----------------------------------------------------------------------------

auto DataMgrFileWrite::write(
    SmBufEx::const_iterator buf_begin,
    SmBufEx::const_iterator buf_end,
    const size_t & position) -> ssize_t
{
  // Check stream opened
  if(!active())
  {
    const std::string tmp_msg{"File output stream not active"};
    L_ERR(tmp_msg);
    set_error_if_first(0, tmp_msg);
    return -1;
  }

  if(auto buf_size=std::distance(buf_begin, buf_end); buf_size > 0)
  {
    ssize_t begin_pos=fs_.tellp();
    if(begin_pos != (ssize_t)position)
    {
      L_WRN("Change write position "+std::to_string(fs_.tellp())+
            " -> "+std::to_string(position));
      fs_.seekp(position);
    }

    begin_pos=fs_.tellp();
    if(fs_.tellp() != (ssize_t)position)
    {
      L_ERR("File stream wrong seek position "+std::to_string(position));
      set_error_if_first(0, "Server write stream seek failed");
      return -1;
    }

    fs_.write(& * buf_begin, buf_size);
    ssize_t end_pos = fs_.tellp();
    if(end_pos < 0)
    {
      L_ERR("File stream wrong write at pos "+std::to_string(position));
      set_error_if_first(0, "Server write stream failed - no writed data");
      return -1;
    }

    return end_pos - begin_pos;
  }



  L_WRN("Nothing to write (no data)");
  return 0;
}

// -----------------------------------------------------------------------------

auto DataMgrFileWrite::read(
    SmBufEx::iterator buf_begin,
    SmBufEx::iterator buf_end,
    const size_t & position) -> ssize_t
{
  throw std::runtime_error(
      "Wrong use method (fail operation 'read' on output stream");
}

//------------------------------------------------------------------------------

} // namespace tftp
