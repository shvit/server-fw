/**
 * \file tftp_data_mgr.h
 * \brief Data manager class header
 *
 *  Data manager class header
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTP_DATA_MGR_H_
#define SOURCE_TFTP_DATA_MGR_H_

#include <fstream>
#include <experimental/filesystem>

#include "tftpCommon.h"
#include "tftpBase.h"

using namespace std::experimental;

namespace tftp
{

/// Alias for filesystem::path
using Path = filesystem::path;

/// Type of callback function when set error
using fSetError = std::function<void(const uint16_t, std::string_view)>;

namespace constants
{
  /// Template for match MD5 by regex
  const std::string regex_template_md5{"([a-fA-F0-9]{32})"};
}

// -----------------------------------------------------------------------------

/** \brief Data manage class
 *
 * Class with file stream operations.
 * Create/close/read/write streams.
 * Check root server directory.
 * In future: Check Firebird connection
 */

class DataMgr: public Base
{
protected:

  // Processing info
  SrvReq        request_type_; ///< Request type
  std::ifstream file_in_;      ///< Input file stream
  std::ofstream file_out_;     ///< Output file stream
  size_t        file_size_;    ///< File size
  fSetError     set_error_;    ///< Callback error parsing to top level

  /** \brief Check Firebird
   *
   *  Now disabled. Will work in future
   *  /return True if Firebird client_library/database exist, else - false
   */
  bool check_fb();

  /** /brief Check opened files
   *
   *  Check input or output stream.
   *  /return True if exist opened/working streams, else - false
   */
  bool active_files() const;

  /** \brief Check opened Firebird connection
   *
   *   Now disabled. Will work in future
   *   /return True if Firebird connection opened, else - false
   */
  bool active_fb_connection();

  /** Check requested value is md5 sum
   *
   *  Match by regex used 'regex_template_md5'
   *  /return True if md5, else - false
   */
  bool match_md5(const std::string & val) const;

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

  /**  Constructor
   */
  DataMgr();
  DataMgr(DataMgr &&) = default;

  DataMgr & operator=(DataMgr &&) = default;

  /** Destructor
   */
  virtual ~DataMgr();

  /** Check active (opened streams or Firebird connection)
   */
  bool active();

  /**  Initialize instance
   *  \param [in] request_type Type tftp request
   *  \param [in] fname Requested file name
   *  \return True if initialize success, else - false
   */
  bool init(SrvReq request_type, const std::string & fname);

  /** \brief Pull data from network (receive)
   *
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
   *  \param [in] position Position received block
   *  \return 0 on success, -1 on error
   */
  ssize_t rx(Buf::iterator buf_begin,
             Buf::iterator buf_end,
             const Buf::size_type position);

  /** \brief Push data to network (transmit)
   *
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
   *  \param [in] position Position transmitted block
   *  \return Processed size, -1 on error
   */
  ssize_t tx(Buf::iterator buf_begin,
             Buf::iterator buf_end,
             const Buf::size_type position);

  /**  Close all opened steams
   */
  void close();

  /** \brief Forward error to top level
   *
   *  Use property set_error_ as callback function, if property set
   *  \param [in] e_cod Error code
   *  \param [in] e_msg Error message
   */
  void set_error_if_first(
      const uint16_t e_cod,
      std::string_view e_msg) const;

  /** \brief Check directory exist
   *
   *  Common used method
   *  \param [in] chk_dir Path
   *  \return True if exist, or false
   */
  //bool check_directory(std::string_view chk_dir) const;

  friend class Session;
};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_DATA_MGR_H_ */
