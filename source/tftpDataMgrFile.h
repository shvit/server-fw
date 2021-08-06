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

#include <fstream>
#include <experimental/filesystem>
//#include <experimental/bits/fs_fwd.h>

#include "tftpCommon.h"
//#include "tftpBase.h"
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
class DataMgrFile: public DataMgr, public SrvBase
{
protected:
  Path          filename_; ///< File path with name; constructed after init()
  std::ifstream file_in_;  ///< Input file stream
  std::ofstream file_out_; ///< Output file stream

  /** \brief Recursive search file by md5 in directory
   *
   *  If finded OK, then open input file stream
   *  \param [in] path Root search directory
   *  \param [in] md5sum Sum of MD5
   *  \return Tuple<found/not found; Path to real file>
   */
  auto search_by_md5(
      const Path & path,
      std::string_view md5sum)
          -> std::tuple<bool, Path>;

  /** \brief Recursive search file by md5 in ALL directories
   *
   *  If finded OK, then open input file stream
   *  Used main server directory and search directories
   *  \param [in] path Root search directory
   *  \param [in] md5sum Sum of MD5
   *  \return Tuple<found/not found; Path to real file>
   */
  auto full_search_md5(std::string_view md5sum)
      -> std::tuple<bool, Path>;

  /** \brief Recursive search file by name in ALL directories
   *
   *  If finded OK, then open input file stream
   *  Used main server directory and search directories
   *  \param [in] name Root search directory
   *  \return Tuple<found/not found; Path to real file>
   */
  auto full_search_name(std::string_view name)
      -> std::tuple<bool, Path>;

public:

  /** \brief Default constructor
   *
   *  No need public construct. Construct allowed only from Session
   */
  DataMgrFile();

  DataMgrFile(const DataMgrFile &) = delete; ///< Deleted/unused

  DataMgrFile(DataMgrFile &) = delete; ///< Deleted/unused

  DataMgrFile(DataMgrFile &&) = delete; ///< Deleted/unused

  DataMgrFile & operator=(const DataMgrFile &) = delete; ///< Deleted/unused

  DataMgrFile & operator=(DataMgrFile &) = delete; ///< Deleted/unused

  DataMgrFile & operator=(DataMgrFile &&) = delete; ///< Deleted/unused

  /** Destructor
   */
  virtual ~DataMgrFile() override;

  /** Check active (opened stream)
   *
   *  Overrided virtual method for file streams
   */
  virtual bool active() const override;

  /** \brief Initialize streams
   *
   *  Overrided virtual method for file streams
   *  \param [in] sett Settings of tftp server
   *  \param [in] cb_error Callback for error forward
   *  \param [in] opt Options of tftp protocol
   *  \return True if initialize success, else - false
   */
  virtual bool init(
      SrvBase & sett,
      fSetError cb_error,
      const Options & opt) override;

  /** \brief Pull data from network (receive)
   *
   *  Overrided virtual method for file streams
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position received block
   *  \return 0 on success, -1 on error
   */
  virtual auto write(
      SmBufEx::const_iterator buf_begin,
      SmBufEx::const_iterator buf_end,
      const size_t & position) -> ssize_t override;

  /** \brief Push data to network (transmit)
   *
   *  Overrided virtual method for file streams
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position transmitted block
   *  \return Processed size, -1 on error
   */
  virtual auto read(
      SmBufEx::iterator buf_begin,
      SmBufEx::iterator buf_end,
      const size_t & position) -> ssize_t override;

  /**  Close all opened steams
   *
   *  Overrided virtual method for file streams
   */
  virtual void close() override;
};

//##############################################################################

namespace ext
{

class DataMgrFile: public DataMgr//, public Logger
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

  //[[derrecated]]
  DataMgrFile();

  DataMgrFile(
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

} // namespace ext

} // namespace tftp

#endif /* SOURCE_TFTP_DATA_MGR_H_ */
