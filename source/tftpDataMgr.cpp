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

#include "tftpDataMgr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

DataMgr::DataMgr():
    Base(),
    request_type_{SrvReq::unknown},
    file_in_{},
    file_out_{},
    set_error_{nullptr}
{
}

// -----------------------------------------------------------------------------

DataMgr::~DataMgr()
{
}

// -----------------------------------------------------------------------------

bool DataMgr::check_fb()
{
  // TODO: Check FB connection

  return false;
}

// -----------------------------------------------------------------------------

ssize_t DataMgr::rx(
    Buf::iterator buf_begin,
    Buf::iterator buf_end,
    const Buf::size_type position)
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
      if(file_out_.tellp() != (ssize_t)position)
      {
        L_WRN("Change write position "+std::to_string(file_out_.tellp())+
              " -> "+std::to_string(position));
        file_out_.seekp(position);
      }
      file_out_.write(& *buf_begin, buf_size);
    }
  }
  else
  {
    L_ERR("File stream not opened");
    set_error_if_first(0, "Server write stream not opened");
    return -1;
  }

  return 0;

}

// -----------------------------------------------------------------------------

ssize_t DataMgr::tx(
    Buf::iterator buf_begin,
    Buf::iterator buf_end,
    const Buf::size_type position)
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

    if(file_in_.tellg() != (ssize_t)position)
    {
      L_WRN("Change read position "+std::to_string(file_in_.tellg())+
            " -> "+std::to_string(position));
      file_in_.seekg(position);
    }
    auto ret_size = static_cast<ssize_t>(file_size_) - (ssize_t)position;
    if(ret_size > 0)
    {
      file_in_.read(& *buf_begin, buf_size);
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

bool DataMgr::active_fb_connection()
{
  // TODO: check active FB connection

  return false;
}

// -----------------------------------------------------------------------------

bool DataMgr::active_files() const
{
  return ((request_type_ == SrvReq::read)  && file_in_ .is_open()) ||
         ((request_type_ == SrvReq::write) && file_out_.is_open());
}

// -----------------------------------------------------------------------------

bool DataMgr::active()
{
  return active_fb_connection() || active_files();
}

// -----------------------------------------------------------------------------

bool DataMgr::match_md5(const std::string & val) const
{
  std::regex regex_md5_pure(constants::regex_template_md5);
  std::smatch sm;

  return std::regex_search(val, sm, regex_md5_pure) &&
         (sm.prefix().str().size() == 0U) &&
         (sm.suffix().str().size() == 0U);

}

// -----------------------------------------------------------------------------

bool DataMgr::init(
    SrvReq request_type,
    const std::string & req_fname)
{
  request_type_ = request_type;

  bool ret = false;
  Path processed_file;

  // 1 Processed in Firebird (fake - skip)
  if(check_fb())
  {
    // TODO: Search in FB

    if(ret) return true;
  }

  // 2 Processed in filesystem
  switch(request_type_)
  {
    case SrvReq::read:
      // ... try find by md5
      if(match_md5(req_fname)) // name is md5 sum ?
      {
        L_INF("Match file as pure md5 request");

        std::tie(ret, processed_file) = full_search_md5(req_fname);
        if(ret)
        {
          L_INF("Find file via his md5 sum '"+processed_file.string()+"'");
        }
      }

      // ... Try find by filename
      if(!ret)
      {
        std::tie(ret, processed_file) = full_search_name(req_fname);
        if(ret)
        {
          L_INF("Find file via his name '"+processed_file.string()+"'");
        }
      }

      // ... Check result
      if(!ret)
      {
        L_ERR("File not found '" + req_fname + "'");
        set_error_if_first(1U, "File not found");
      }

      // ... Try open
      if(ret)
      {
        std::ios_base::iostate ex_mask = file_in_.exceptions() | std::ios::failbit;
        file_in_.exceptions(ex_mask);
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
      processed_file /= req_fname;
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
        std::ios_base::iostate ex_mask = file_out_.exceptions() | std::ios::failbit;
        file_out_.exceptions(ex_mask);
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

void DataMgr::close()
{
  // TODO: close FB connection

  // files
  if(file_in_ .is_open()) file_in_ .close();
  if(file_out_.is_open()) file_out_.close();

}

// -----------------------------------------------------------------------------

auto DataMgr::search_by_md5(
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
      std::regex regex_md5_sum("([a-f0-9]{32})");
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

auto DataMgr::full_search_md5(std::string_view md5sum)
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

auto DataMgr::full_search_name(std::string_view name)
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

void DataMgr::set_error_if_first(
    const uint16_t e_cod,
    std::string_view e_msg) const
{
  if(set_error_) set_error_(e_cod, e_msg);
}

// -----------------------------------------------------------------------------

} // namespace tftp
