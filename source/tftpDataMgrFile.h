/**
 * \file tftpDataMgrFile.h
 * \brief Data manager class for files header
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

#ifndef SOURCE_TFTP_DATA_MGR_H_
#define SOURCE_TFTP_DATA_MGR_H_

#include <experimental/filesystem>
//#include <experimental/bits/fs_fwd.h>

#include "tftpCommon.h"
#include "tftpDataMgr.h"
#include "tftpLogger.h"

using namespace std::experimental;

namespace tftp
{

/// Alias for filesystem::path
using Path = filesystem::path;

using Perms = filesystem::perms;

// -----------------------------------------------------------------------------

/** \brief Data manage streams for files
 */
class DataMgrFile: public DataMgr, public Logger
{
protected:

  Path filename_; ///< File path with name; constructed after init()

  VecStr dirs_; ///< Directories for search file

  /** \brief Recursive search file by md5 in directory
   *
   *  On success set filename_
   *  TODO: MD5-multifile parsing
   *  \param [in] path Root search directory
   *  \param [in] md5sum Sum of MD5
   *  \return True on success, else - false
   */
  bool search_rec_by_md5(
      const Path & path,
      std::string_view md5sum);

  /** \brief Recursive search file by name in directory
   *
   *  On success set filename_
   *  \param [in] path Root search directory
   *  \param [in] name Filename
   *  \return True on success, else - false
   */
  bool search_rec_by_name(
      const Path & path,
      std::string_view name);

public:

  //[[deprecated]]
  //DataMgrFile();

  DataMgrFile(
      fLogMsg logger,
      fSetError err_setter,
      std::string_view new_filename,
      VecStr && new_dirs);

  /** \brief Full search file in ALL directories, recursive
   *
   *  Search will use all server directories
   *  Each directory use recursive
   *  Each directory find by filename then fing by md5sum
   *  Use for readed files
   *  On success set filename_
   *  \param [in] name Filename or MD5 sum
   *  \return True on success, else - false
   */
  bool full_search(std::string_view name);

  /** \brief Recursive search file by name in directories
   *
   *  Search will only main server directory
   *  No recursion
   *  Use for writed files
   *  On success set filename_
   *  \param [in] name Search directory
   *  \param [in] only_root Search only first (main) directory
   *  \return True on success, else - false
   */
  bool search_root_by_name(
      std::string_view name,
      bool only_root=false);


};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_DATA_MGR_H_ */
