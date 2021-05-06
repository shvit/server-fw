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
    request_name_{""},
    file_in_{},
    file_out_{},

    hash_{""},
    ifs_{nullptr},
    ifs_mode_{0},
    ifs_size_{0},
    ofs_{},
    set_error_{nullptr}
{
}

// -----------------------------------------------------------------------------

DataMgr::~DataMgr()
{
}

// -----------------------------------------------------------------------------
bool DataMgr::check_directory(std::string_view chk_dir) const
{
  if(!chk_dir.size()) return false;

  std::string path{chk_dir};
  struct stat sb;
  return (stat(path.c_str(), & sb) == 0) && S_ISDIR(sb.st_mode);
}

// -----------------------------------------------------------------------------

bool DataMgr::check_root_dir()
{
  if(check_directory(get_root_dir()))
  {
    L_DBG("Root directory is fine");
    return true;
  }
  else
  {
    L_WRN("Wrong root directory");
    return false;
  }
}

// -----------------------------------------------------------------------------

bool DataMgr::check_fb()
{
  std::string fb_lib_path{get_lib_dir()+get_lib_name_fb()};

  auto h = dlopen(fb_lib_path.c_str(), RTLD_NOW);

  bool ret = (h != nullptr);

  if(!ret)
  {
    L_DBG("Not found firebird client library '"+fb_lib_path+"'");
    return false;
  }

  L_DBG("Found firebird client library '"+fb_lib_path+"'");

  // TODO: check lib version

  auto lk = begin_shared(); // read lock

  ret = settings_->db.size() &&
        settings_->user.size() &&
        settings_->dialect;
  if(!ret)
  {
    L_DBG("No account information to access Firebird (db/user/dialect)");
    return false;
  }

  // TODO: check db
  // TODO: table struct

  dlclose(h);

  ret=false; // TODO: remove this hack to enable FB

  return ret;
}

// -----------------------------------------------------------------------------

ssize_t DataMgr::rx(
    Buf::iterator buf_begin,
    Buf::iterator buf_end,
    const Buf::size_type position)
{
  if(request_type_ != SrvReq::write)
  {
    L_ERR("Need request type 'write' (really '"+
          std::string(to_string(request_type_))+"')");
    set_error_if_first(0, "Wrong request type");
    return -1;
  }

  if(ofs_.is_open())
  {
    if(auto buf_size=std::distance(buf_begin, buf_end); buf_size > 0)
    {
      if(ofs_.tellp() != (ssize_t)position)
      {
        L_WRN("Change write position "+std::to_string(ofs_.tellp())+
              " -> "+std::to_string(position));
        ofs_.seekp(position);
      }
      L_DBG("Try store block (buf size "+std::to_string(buf_size)+
            "; position "+std::to_string(position)+")");
      ofs_.write(& *buf_begin, buf_size);
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
    L_ERR("Need request type 'read' (really '"+
              std::string(to_string(request_type_))+"')");
    set_error_if_first(0, "Wrong request");
    return -1;
  }

  //TODO:
  if(ifs_ != nullptr)
  {
    auto buf_size = std::distance(buf_begin, buf_end);
    L_DBG("Generate block (buf size "+std::to_string(buf_size)+
          "; position "+std::to_string(position)+")");

    if(ifs_->tellg() != (ssize_t)position)
    {
      L_WRN("Change read position "+std::to_string(ifs_->tellg())+
            " -> "+std::to_string(position));
      ifs_->seekg(position);
    }
    auto ret_size = static_cast<ssize_t>(ifs_size_) - (ssize_t)position;
    if(ret_size > 0)
    {
      ifs_->read(& *buf_begin, buf_size);
      if(ret_size > buf_size) ret_size = buf_size;
    }
    return ret_size;
  }

  // not opened
  L_ERR("File stream not opened");
  set_error_if_first(0, "Server read stream not opened (file not found)");
  return -1;
}


// -----------------------------------------------------------------------------

bool DataMgr::active_fb_connection()
{
  return false;
}

// -----------------------------------------------------------------------------

bool DataMgr::active_files() const
{
  return ((request_type_ == SrvReq::read)  && ifs_) ||
         ((request_type_ == SrvReq::write) && ofs_.is_open());
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
  request_name_.assign(req_fname);

  bool ret = false;
  Path processed_file;

  // 1 Processed in Firebird (fake - skip)
  if(check_fb())
  {

    return true;
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
      if(!ret)
      {
        std::ios_base::iostate ex_mask = file_out_.exceptions() | std::ios::failbit;
        file_out_.exceptions(ex_mask);
        try
        {
          file_out_.open(processed_file, std::ios_base::out | std::ios::binary);
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

  //return ret;


/*
  // 2 Search in md5
  if(request_type_ == SrvReq::read)
  {
    std::regex regex_md5_pure("([a-f0-9]{32})");
    std::smatch sm;
    if(std::regex_search(request_name_,
                         sm,
                         regex_md5_pure) &&
       (sm.prefix().str().size()==0) &&
       (sm.suffix().str().size()==0))
    {
      L_INF("Match file as pure md5 request");

      //if(recursive_search_by_md5(get_root_dir()))

      for(size_t iter=0; iter < settings_->backup_dirs.size(); ++iter)
      {
        if(recursive_search_by_md5(settings_->backup_dirs[iter])) break;
      }
    }
  }

  // 3 Check input/output files
  Path p{get_root_dir()};
  p /= Path{request_name_}.filename();

  if((request_type_ == SrvReq::read) &&
      filesystem::exists(p))
  {
    file_in_.open(p, std::ios_base::in | std::ios::binary | std::ios::ate);

    if(file_in_.is_open())
    {
      L_ERR("File not opened '" + request_name_ + "'");
      set_error_if_first(0, "File not opened");
      close();
      return false;
    }
  }

  if((request_type_ == SrvReq::write) &&
      filesystem::exists(p))
  {
  }
*/


  if(check_root_dir())
  {
    //std::string full_fname{get_root_dir() + request_name_};

    switch(request_type_)
    {
      case SrvReq::read:
        switch(ifs_mode_ = is_md5())
        {
          case 0: //request_name_ = get_root_dir() + request_name_;
          case 1:
            ifs_ = new std::ifstream;
            static_cast<std::ifstream *>(ifs_)->
                open(request_name_,
                     std::ios_base::in | std::ios::binary | std::ios::ate);

            if(!static_cast<std::ifstream *>(ifs_)->is_open())// if not opened
            {
              L_ERR("File not opened '" + request_name_ + "'");
              set_error_if_first(0, "File not opened");
              close();
              return false;
            }
            ifs_size_ = ifs_->tellg();
            ifs_->seekg(0);
            break;
          case 2: // *.md5 file
            {
              ifs_ = new std::istringstream{hash_+" fake_filename\n"};
              ifs_size_ = hash_.size() + 16;
            }
            break;
          default:
            break;
        }
        break;
      case SrvReq::write:
	      request_name_ = get_root_dir() + request_name_;
        ofs_.open(request_name_, std::ios_base::out | std::ios::binary);
        if(!ofs_.is_open()) // if not opened
        {
          L_ERR("File not opened '" + request_name_ + "'");
          set_error_if_first(0, "File not opened");
          return false;
        }
        break;
      default:
        L_WRN("Wrong request type '" +
              std::string(to_string(request_type_)) + "'");
        return false;
    }
    return true;
  }

  L_ERR("No access to data storage (directory,firebird)");
  set_error_if_first(0, "No access to data storage (directory, firebird)");
  return false;
}

// -----------------------------------------------------------------------------

void DataMgr::close()
{
 // TODO: close connection

 // files
 if(ifs_)
 {
   switch(ifs_mode_)
   {
     case 0:
     case 1:
       static_cast<std::ifstream *>(ifs_)->close();
       break;
     default:
       break;
   }
   delete ifs_;
 }

 ifs_ = nullptr;
 if(ofs_.is_open()) ofs_.close();

}

// -----------------------------------------------------------------------------

int DataMgr::is_md5()
{
  std::regex regex_md5_pure("([a-f0-9]{32})");
  std::regex regex_md5_file("([a-f0-9]{32})([.][mM][dD][5])");
  std::smatch sm;
  int ret=0;

  // 1 Check files <md5 hash>.md5
  if(std::regex_search(request_name_,
                       sm,
                       regex_md5_file) &&
     (sm.size()==3) &&
     (sm.prefix().str().size()==0) &&
     (sm.suffix().str().size()==0))
  {
    hash_ = sm[1].str();
    L_INF("Match file as *.md5 sum");
    ret=2;
  }
  else
  // 2 Checkfiles  <md5 hash>
  if(std::regex_search(request_name_,
                       sm,
                       regex_md5_pure) &&
     (sm.prefix().str().size()==0) &&
     (sm.suffix().str().size()==0))
  {
    hash_ = sm[1].str();
    L_INF("Match file as pure md5 request");
    ret=1;

    auto lk = begin_shared(); // read lock (recursive!)

    for(size_t iter=0; iter < settings_->backup_dirs.size(); ++iter)
    {
      if(recursive_search_by_md5(settings_->backup_dirs[iter])) break;
    }
  }
  else
  // 3 Simple try search file - nothing do
  {
    request_name_ = get_root_dir() + request_name_;
  }
  return ret;
}

// -----------------------------------------------------------------------------

bool DataMgr::recursive_search_by_md5(const std::string & path)
{
  bool ret = false;
  if (auto dir = opendir(path.c_str()))
  {
    while (auto f = readdir(dir))
    {
      if((f->d_name[0] == '.') &&
         (f->d_name[1] == '\0'))  // self dir
      {
        continue;
      }
      if((f->d_name[0] == '.') &&
         (f->d_name[1] == '.') &&
         (f->d_name[2] == '\0')) // up dir
      {
        continue;
      }

      std::string curr_name{std::string(f->d_name)};
      std::string full_name{path};
      if(full_name.size() && (full_name[full_name.size()-1] != '/'))
      {
        full_name.append("/");
      }
      full_name.append(curr_name);

      if(f->d_type == DT_DIR)
      {
        ret = recursive_search_by_md5(full_name);
      }
      if(f->d_type == DT_REG)
      {
        std::regex regex_md5_file("(.[mM][dD]5)");
        std::smatch sm_file;

        if(std::regex_search(curr_name, sm_file, regex_md5_file))
        {
          std::ifstream file_md5;
          file_md5.open(full_name, std::ios_base::in);

          for(Buf arr(1024,0);
              file_md5.getline(arr.data(), arr.size(), '\n'); )
          {
            std::string line{arr.data()};
            std::regex regex_md5_sum("([a-f0-9]{32})");
            std::smatch sm_sum;
            if(std::regex_search(line, sm_sum, regex_md5_sum))
            {
              if(sm_sum[1].str() == hash_)
              {
                request_name_ = path;
                if(request_name_.size() && (request_name_[request_name_.size()-1] != '/'))
                {
                  request_name_.append("/");
                }
                request_name_.append(sm_file.prefix().str());
                ret = true;
                L_INF("Found file '"+request_name_+"'");
              }
            }
            break; // fuck long files
          }
          file_md5.close();
        }
      }
      if(ret) break;
    }
    closedir(dir);
  }

  return ret;
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
    if(ext == "md5")
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
