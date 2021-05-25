/**
 * \file tftp_data_mgr.cpp
 * \brief Data manager class module
 *
 *  Data manager class module
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <dirent.h>
#include <linux/limits.h>
#include <regex>
#include <sys/stat.h>
#include <dlfcn.h>

#include "tftpDataMgrFile.h"
#include "tftpOptions.h"
#include "tftpSmBufEx.h"

namespace tftp
{

// -----------------------------------------------------------------------------

DataMgrFile::DataMgrFile():
    IDataMgr(),
    Base(),
    file_in_{},
    file_out_{}
{
}

// -----------------------------------------------------------------------------

DataMgrFile::~DataMgrFile()
{
}

// -----------------------------------------------------------------------------

bool DataMgrFile::active() const
{
  return ((request_type_ == SrvReq::read)  && file_in_ .is_open()) ||
         ((request_type_ == SrvReq::write) && file_out_.is_open());
}

// -----------------------------------------------------------------------------

bool DataMgrFile::init(
    pSettings & sett,
    fSetError cb_error,
    const Options & opt)
{
  settings_ = sett;
  set_error_ = cb_error;
  request_type_ = opt.request_type();

  bool ret = false;
  Path processed_file;

  switch(request_type_)
  {
    case SrvReq::read:
      // ... try find by md5
      if(match_md5(opt.filename())) // name is md5 sum ?
      {
        L_INF("Match file as pure md5 request");

        std::tie(ret, processed_file) = full_search_md5(opt.filename());
        if(ret)
        {
          L_INF("Find file via his md5 sum '"+processed_file.string()+"'");
        }
      }

      // ... Try find by filename
      if(!ret)
      {
        std::tie(ret, processed_file) = full_search_name(opt.filename());
        if(ret)
        {
          L_INF("Find file via his name '"+processed_file.string()+"'");
        }
      }

      // ... Check result
      if(!ret)
      {
        L_ERR("File not found '" + opt.filename() + "'");
        set_error_if_first(1U, "File not found");
      }

      // ... Try open
      if(ret)
      {
        auto backup_val = file_in_.exceptions();
        file_in_.exceptions(std::ios::failbit);
        try
        {
          file_in_.open(processed_file, std::ios_base::in | std::ios::binary);
        }
        catch (const std::system_error & e)
        {
          ret = false;
          L_ERR(std::string{"Error: "}+e.what()+" ("+
                std::to_string(e.code().value())+")");
          set_error_if_first(0U, e.what());
        }
        file_in_.close();
        file_in_.exceptions(backup_val);
        file_in_.open(processed_file, std::ios_base::in | std::ios::binary);
      }

      // ... Other
      if(ret)
      {
        file_size_ = filesystem::file_size(processed_file);
      }

      break;
    case SrvReq::write:
      // ... Check file exist
      processed_file = get_root_dir();
      processed_file /= opt.filename();
      ret = !filesystem::exists(processed_file);

      // ... Result is wrong (file exist)
      if(!ret)
      {
        L_ERR("File already exists '"+processed_file.string()+"'");
        set_error_if_first(6U, "File already exists");
      }

      // ... File not exist
      if(ret)
      {
        file_out_.exceptions(file_out_.exceptions() | std::ios::failbit);
        try
        {
          file_out_.open(processed_file, std::ios_base::out | std::ios::binary);
          file_out_.write(nullptr, 0U);
        }
        catch (const std::system_error & e)
        {
          ret = false;
          L_ERR(std::string{"Error: "}+e.what()+" ("+
                std::to_string(e.code().value())+")");
          set_error_if_first(0U, e.what());
        }

      }
      break;
    default:
      return false;
  }

  L_INF("Data manager initialise is "+(ret ? "SUCCESSFUL" : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

auto DataMgrFile::write(
    SmBufEx::const_iterator buf_begin,
    SmBufEx::const_iterator buf_end,
    const size_t & position) -> ssize_t
{
  if(request_type_ != SrvReq::write)
  {
    throw std::runtime_error(
        "Wrong use method (can't use rx() when request type != write");
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
        set_error_if_first(0, "Server write stream seek failed");
        return -1;
      }

      file_out_.write(& * buf_begin, buf_size);
      ssize_t end_pos = file_out_.tellp();
      if(end_pos < 0)
      {
        L_ERR("File stream wrong write at pos "+std::to_string(position));
        set_error_if_first(0, "Server write stream failed - no writed data");
        return -1;
      }

      return end_pos - begin_pos;
    }
  }
  else
  {
    L_ERR("File stream not opened");
    set_error_if_first(0, "Server write stream not opened");
    return -1;
  }

  L_WRN("Nothing to write (no data)");
  return 0;
}

// -----------------------------------------------------------------------------

auto DataMgrFile::read(
    SmBufEx::iterator buf_begin,
    SmBufEx::iterator buf_end,
    const size_t & position) -> ssize_t
{
  if(request_type_ != SrvReq::read)
  {
    throw std::runtime_error(
        "Wrong use method (can't use tx() when request type != read");
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
  set_error_if_first(0, "Server read stream not opened");
  return -1;
}

// -----------------------------------------------------------------------------

void DataMgrFile::close()
{
  if(file_in_ .is_open()) file_in_ .close();
  if(file_out_.is_open()) file_out_.close();
}

// -----------------------------------------------------------------------------

auto DataMgrFile::search_by_md5(
    const Path & path,
    std::string_view md5sum) -> std::tuple<bool, Path>
{
  for(auto & curr_iter : filesystem::recursive_directory_iterator(path))
  {
    Path curr=curr_iter.path();
    std::string ext{curr.extension()};
    do_lower(ext);
    if(ext == ".md5")
    {
      // read first line
      std::ifstream file_md5{curr, std::ios_base::in};
      std::string line1(2048,0);
      file_md5.getline(line1.data(), line1.size(), '\n');

      // check md5
      std::regex regex_md5_sum(constants::regex_template_md5);
      std::smatch sm_sum;
      if(std::regex_search(line1, sm_sum, regex_md5_sum))
      {
        if(sm_sum[1].str() == md5sum) // md5 matched!
        {
          L_DBG("Match md5 sum at file '"+curr.string()+"'");

          // Try same filename without extension
          curr.replace_extension("");
          if(filesystem::exists(curr)) return {true, curr};

          // Try filename from md5 file
          if(sm_sum.suffix().str().size())
          {
            curr.replace_filename(sm_sum.suffix().str());
            if(filesystem::exists(curr)) return {true, curr};
          }

          L_DBG("Matched MD5 file not found!");
        }
      }
    }
  }

  return {false, Path()};
}

// -----------------------------------------------------------------------------

auto DataMgrFile::full_search_md5(std::string_view md5sum)
    -> std::tuple<bool, Path>
{
  // Search in main dir
  Path curr_dir{get_root_dir()};
  if(filesystem::exists(curr_dir) &&
     filesystem::is_directory(curr_dir))
  {
    auto [res, file] = search_by_md5(curr_dir, md5sum);
    if(res) return {true, file};
  }

  // Search in secondary dirs
  for(const auto & path : get_serach_dir())
  {
    Path curr_dir{path};
    if(filesystem::exists(curr_dir) &&
       filesystem::is_directory(curr_dir))
    {
      auto [res, file] = search_by_md5(curr_dir, md5sum);
      if(res) return {true, file};
    }
  }

  return {false, Path()};
}

// -----------------------------------------------------------------------------

auto DataMgrFile::full_search_name(std::string_view name)
    -> std::tuple<bool, Path>
{
  // Search in main dir
  Path curr_file{get_root_dir()};
  curr_file /= name;
  if(filesystem::exists(curr_file) &&
     filesystem::is_regular_file(curr_file))
  {
    return {true, curr_file};
  }

  // Search in secondary dirs
  for(const auto & path : get_serach_dir())
  {
    Path curr_file{path};
    curr_file /= name;
    if(filesystem::exists(curr_file) &&
       filesystem::is_regular_file(curr_file))
    {
      return {true, curr_file};
    }
  }

  return {false, Path()};
}
// -----------------------------------------------------------------------------

} // namespace tftp
