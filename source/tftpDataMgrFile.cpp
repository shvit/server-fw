/**
 * \file tftpDataMgrFile.cpp
 * \brief Data manager class for files module
 *
 *  Data manager for files
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <fstream>
#include <dirent.h>
#include <linux/limits.h>
#include <regex>
#include <sys/stat.h>
#include <dlfcn.h>
#include <unistd.h>
#include <system_error>

#include "tftpDataMgrFile.h"
#include "tftpOptions.h"
#include "tftpSmBufEx.h"

namespace tftp
{

// -----------------------------------------------------------------------------

DataMgrFile::DataMgrFile(
    fLogMsg logger,
    fSetError err_setter):
        DataMgr(err_setter),
        Logger(logger),
        filename_{}
{
  //set_error_ = err_setter;
}

// -----------------------------------------------------------------------------

bool DataMgrFile::search_rec_by_md5(
    const Path & path,
    std::string_view md5sum)
{
  for(auto & curr_iter : filesystem::recursive_directory_iterator(path))
  {
    Path curr=curr_iter.path();
    std::string ext{curr.extension()};
    do_lower(ext);
    if(ext != ".md5") continue;

    // read first line
    std::ifstream file_md5{curr, std::ios_base::in};
    std::string line1(4096,0);
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
        if(filesystem::exists(curr))
        {
          swap(filename_,curr);
          return true;
        }

        // Try filename from md5 file
        if(sm_sum.suffix().str().size())
        {
          curr.replace_filename(sm_sum.suffix().str());
          if(filesystem::exists(curr))
          {
            swap(filename_,curr);
            return true;
          }
        }

        L_DBG("Matched MD5 file not found!");
      }
    }
  }

  return false;
}

// -----------------------------------------------------------------------------

bool DataMgrFile::search_rec_by_name(
    const Path & path,
    std::string_view name)
{
  for(auto & curr_iter : filesystem::recursive_directory_iterator(path))
  {
    Path curr=curr_iter.path();

    if(curr.filename() == name)
    {
      swap(filename_, curr);
      L_DBG("Matched file found ("+curr.string()+")");
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------

bool DataMgrFile::full_search(
    std::string_view name,
    std::string_view root_dir,
    const VecStr & search_dirs)
{
  filename_.clear();

  if(search_rec_by_name(root_dir, name)) return true;

  if(search_rec_by_md5(root_dir, name)) return true;

  for(const auto & path : search_dirs)
  {
    Path curr_dir{path};
    if(filesystem::exists(curr_dir) &&
       filesystem::is_directory(curr_dir))
    {
      if(search_rec_by_name(curr_dir, name)) return true;

      if(search_rec_by_md5(curr_dir, name)) return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
/*
bool DataMgrFile::search_root_by_name(
    std::string_view name,
    bool only_root)
{
  filename_.clear();
  if(dirs_.size())
  {
    Path curr_file{dirs_[0U]};
    curr_file /= name;
    if(filesystem::exists(curr_file) &&
       filesystem::is_regular_file(curr_file))
    {
      std::swap(filename_, curr_file);
      return true;
    }
  }

  return false;
}
*/
// -----------------------------------------------------------------------------

auto DataMgrFile::get_filename() const -> const Path &
{
  return filename_;
}

// -----------------------------------------------------------------------------

} // namespace tftp
