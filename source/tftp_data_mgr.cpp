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

#include <algorithm>
#include <dirent.h>
#include <functional>
#include <linux/limits.h>
#include <regex>
#include <sys/stat.h>
#include <dlfcn.h>

#include "tftp_data_mgr.h"

namespace tftp
{

// ----------------------------------------------------------------------------------

DataMgr::DataMgr():
    Base(),
    request_type_{SrvReq::unknown},
    fname_{""},
    hash_{""},
    ifs_{nullptr},
    ifs_mode_{0},
    ifs_size_{0},
    ofs_{},
    set_error_{nullptr}
{
}

// ----------------------------------------------------------------------------------

DataMgr::~DataMgr()
{
}

// ----------------------------------------------------------------------------------
bool DataMgr::check_directory(std::string_view chk_dir) const
{
  if(!chk_dir.size()) return false;

  std::string path{chk_dir};
  //path.push_back('\0');
  struct stat sb;
  return (stat(path.c_str(), & sb) == 0) && S_ISDIR(sb.st_mode);
}

// ----------------------------------------------------------------------------------

bool DataMgr::check_root_dir()
{
  if(check_directory(get_root_dir()))
  {
    LOG(LOG_DEBUG, "Root directory is fine");
    return true;
  }
  else
  {
    LOG(LOG_WARNING, "Wrong root directory");
    return false;
  }
}

// ----------------------------------------------------------------------------------

bool DataMgr::check_fb()
{
  std::string fb_lib_path{get_lib_dir()+get_lib_name_fb()};

  auto h = dlopen(fb_lib_path.c_str(), RTLD_NOW);

  bool ret = (h != nullptr);

  if(!ret)
  {
    LOG(LOG_DEBUG, "Not found firebird client library '"+fb_lib_path+"'");
    return false;
  }

  LOG(LOG_DEBUG, "Found firebird client library '"+fb_lib_path+"'");

  // TODO: check lib version

  auto lk = begin_shared(); // read lock

  ret = (settings_->db.size() && settings_->user.size() && settings_->dialect);
  if(!ret)
  {
    LOG(LOG_DEBUG, "No account information to access Firebird (db/user/dialect)");
    return false;
  }

  // TODO: check db
  // TODO: table struct

  dlclose(h);

  ret=false; // TODO: remove this hack to enable FB

  return ret;
}

// ----------------------------------------------------------------------------------

ssize_t DataMgr::rx(
    Buf::iterator buf_begin,
    Buf::iterator buf_end,
    const Buf::size_type position)
{
  if(request_type_ != SrvReq::write)
  {
    LOG(LOG_ERR, "Need request type 'write' (really '"+std::string(to_string(request_type_))+"')");
    set_error_if_first(0, "Wrong request type");
    return -1;
  }

  if(ofs_.is_open())
  {
    if(auto buf_size=std::distance(buf_begin, buf_end); buf_size > 0)
    {
      if(ofs_.tellp() != (ssize_t)position)
      {
        LOG(LOG_WARNING, "Change write position "+std::to_string(ofs_.tellp())+" -> "+std::to_string(position));
        ofs_.seekp(position);
      }
      LOG(LOG_DEBUG, "Try store block (buf size "+std::to_string(buf_size)+"; position "+std::to_string(position)+")");
      ofs_.write(& *buf_begin, buf_size);
      //ofs_.flush();
    }
  }
  else
  {
    LOG(LOG_ERR, "File stream not opened");
    set_error_if_first(0, "Server write stream not opened");
    return -1;
  }

  return 0;

}

// ----------------------------------------------------------------------------------

ssize_t DataMgr::tx(
    Buf::iterator buf_begin,
    Buf::iterator buf_end,
    const Buf::size_type position)
{
  if(request_type_ != SrvReq::read)
  {
    LOG(LOG_ERR, "Need request type 'read' (really '"+std::string(to_string(request_type_))+"')");
    set_error_if_first(0, "Wrong request");
    return -1;
  }

  //TODO:
  if(ifs_ != nullptr)
  {
    auto buf_size = std::distance(buf_begin, buf_end);
    LOG(LOG_DEBUG, "Generate block (buf size "+std::to_string(buf_size)+"; position "+std::to_string(position)+")");

    if(ifs_->tellg() != (ssize_t)position)
    {
      LOG(LOG_WARNING, "Change read position "+std::to_string(ifs_->tellg())+" -> "+std::to_string(position));
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
  LOG(LOG_ERR, "File stream not opened");
  set_error_if_first(0, "Server read stream not opened (file not found)");
  return -1;
}


// ----------------------------------------------------------------------------------

bool DataMgr::active_fb_connection()
{
  return false;
}

// ----------------------------------------------------------------------------------

bool DataMgr::active_files() const
{
  return ((request_type_ == SrvReq::read)  && ifs_) ||
         ((request_type_ == SrvReq::write) && ofs_.is_open());
}

// ----------------------------------------------------------------------------------

bool DataMgr::active()
{
  return active_fb_connection() || active_files();
}

// ----------------------------------------------------------------------------------

bool DataMgr::init(SrvReq request_type, std::string_view req_fname)
{
  request_type_ = request_type;
  fname_.assign(req_fname);

  if(check_fb())
  {

    return true;
  }

  if(check_root_dir())
  {
    //std::string full_fname{get_root_dir() + fname_};

    switch(request_type_)
    {
      case SrvReq::read:
        switch(ifs_mode_ = is_md5())
        {
          case 0: //fname_ = get_root_dir() + fname_;
          case 1:
            ifs_ = new std::ifstream;
            static_cast<std::ifstream *>(ifs_)->open(fname_, std::ios_base::in | std::ios::binary | std::ios::ate);

            if(!static_cast<std::ifstream *>(ifs_)->is_open()) // if not opened
            {
              LOG(LOG_ERR, "File not opened '" + fname_ + "'");
              set_error_if_first(0, "File not opened");
              close();
              return false;
            }
            ifs_size_ = ifs_->tellg();
            ifs_->seekg(0);
            break;
          case 2: // *.md5 file
            {
              ifs_ = new std::istringstream{hash_+"  fake_filename\n"};
              ifs_size_ = hash_.size() + 16;
            }
            break;
          default:
            break;
        }
        break;
      case SrvReq::write:
	      fname_ = get_root_dir() + fname_;
        ofs_.open(fname_, std::ios_base::out | std::ios::binary);
        if(!ofs_.is_open()) // if not opened
        {
          LOG(LOG_ERR, "File not opened '" + fname_ + "'");
          set_error_if_first(0, "File not opened");
          return false;
        }
        break;
      default:
        LOG(LOG_WARNING, "Wrong request type '" + std::string(to_string(request_type_)) + "'");
        return false;
    }
    return true;
  }

  LOG(LOG_ERR, "No access to data storage (directory,firebird)");
  set_error_if_first(0, "No access to data storage (directory, firebird)");
  return false;
}

// ----------------------------------------------------------------------------------

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

// ----------------------------------------------------------------------------------

int DataMgr::is_md5()
{
  std::regex regex_md5_pure("([a-f0-9]{32})");
  std::regex regex_md5_file("([a-f0-9]{32})([.][mM][dD][5])");
  std::smatch sm;
  int ret=0;

  // 1 Check files <md5 hash>.md5
  if(std::regex_search(fname_,
                       sm,
                       regex_md5_file) &&
     (sm.size()==3) &&
     (sm.prefix().str().size()==0) &&
     (sm.suffix().str().size()==0))
  {
    hash_ = sm[1].str();
    LOG(LOG_INFO, "Match file as *.md5 sum");
    ret=2;
  }
  else
  // 2 Checkfiles  <md5 hash>
  if(std::regex_search(fname_,
                       sm,
                       regex_md5_pure) &&
     (sm.prefix().str().size()==0) &&
     (sm.suffix().str().size()==0))
  {
    hash_ = sm[1].str();
    LOG(LOG_INFO, "Match file as pure md5 request");
    ret=1;

    //if(!recursive_search_by_md5(get_root_dir()))
    //{
    auto lk = begin_shared(); // read lock (recursive!)

    for(size_t iter=0; iter < settings_->backup_dirs.size(); ++iter)
    {
      if(recursive_search_by_md5(settings_->backup_dirs[iter])) break;
    }
    //}
  }
  else
  // 3 Simple try search file - nothing do
  {
    fname_ = get_root_dir() + fname_;
  }
  return ret;
}

// ----------------------------------------------------------------------------------

bool DataMgr::recursive_search_by_md5(const std::string & path)
{
  bool ret = false;
  if (auto dir = opendir(path.c_str()))
  {
    while (auto f = readdir(dir))
    {
      if(f->d_name[0] == '.' && f->d_name[1] == '\0')                        continue; // self dir
      if(f->d_name[0] == '.' && f->d_name[1] == '.' && f->d_name[2] == '\0') continue; // up dir

      std::string curr_name{std::string(f->d_name)};
      std::string full_name{path};
      if(full_name.size() && (full_name[full_name.size()-1] != '/')) full_name.append("/");
      full_name.append(curr_name);

      if(f->d_type == DT_DIR) ret = recursive_search_by_md5(full_name);
      if(f->d_type == DT_REG)
      {
        std::regex regex_md5_file("(.[mM][dD]5)");
        std::smatch sm_file;

        if(std::regex_search(curr_name, sm_file, regex_md5_file))
        {
          std::ifstream file_md5;
          file_md5.open(full_name, std::ios_base::in);

          for (Buf arr(1024,0); file_md5.getline(arr.data(), arr.size(), '\n'); )
          {
            std::string line{arr.data()};
            std::regex regex_md5_sum("([a-f0-9]{32})");
            std::smatch sm_sum;
            if(std::regex_search(line, sm_sum, regex_md5_sum))
            {
              if(sm_sum[1].str() == hash_)
              {
                fname_ = path;
                if(fname_.size() && (fname_[fname_.size()-1] != '/')) fname_.append("/");
                fname_.append(sm_file.prefix().str());
                ret = true;
                LOG(LOG_INFO, "Found file '"+fname_+"'");
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

// ----------------------------------------------------------------------------------

void DataMgr::set_error_if_first(const uint16_t e_cod, std::string_view e_msg) const
{
  if(set_error_) set_error_(e_cod, e_msg);
}

// ----------------------------------------------------------------------------------

} // namespace tftp
