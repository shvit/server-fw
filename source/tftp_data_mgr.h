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
#include <functional>
#include <iostream>
#include <string>

#include "tftp_common.h"
#include "tftpBase.h"


namespace tftp
{

/** \brief Data maange class
 *
 * Class with file stream operations.
 * Create/close/read/write streams.
 * Check root server directory.
 * In future: Check Firebird connection
 *
 */

class data_mgr: public Base
{
protected:
  // Processing info
  srv_req     request_type_; ///< Request type
  std::string fname_;        ///< Processed file name
  std::string hash_;         ///< Hash of file (md5)

  std::basic_istream<char> * ifs_;      ///< Input data stream
  int                        ifs_mode_; ///< Input mode
  ssize_t                    ifs_size_; ///< Input data size (known)
  std::ofstream              ofs_;      ///< Output stream

  /** \brief Callback error parsing to top level
   */
  std::function<void(const uint16_t, std::string_view)> set_error_;

  /** \brief Check root directory
   *
   *  Check path exist and his type is directory.
   *  /return True if root directory OK, else - false
   */
  bool check_root_dir();

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

  /** Check requested filename is md5 sum
   *
   *  Match by regex.
   *  /return 0-no; 1-pure md5; 2-md5 sum file
   *
   */
  int is_md5();

  /** \brief Recursive search file by md5
   *
   *  If finded OK, then fname_ has full filename with path.
   *  \param [in] path Start search directory
   *  \return True if file found, else - false
   */
  bool recursive_search_by_md5(const std::string & path);

public:

  /**  Constructor
   */
  data_mgr();

  /** Destructor
   */
  virtual ~data_mgr();

  /** Check active (opened streams or Firebird connection)
   */
  bool active();

  /**  Initialize instance
   *  \param [in] request_type Type tftp request
   *  \param [in] fname Requested file name
   *  \return True if initialize success, else - false
   */
  bool init(srv_req request_type, std::string_view fname);

  /** \brief Pull data from network (receive)
   *
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
   *  \param [in] position Position received block
   *  \return 0 on success, -1 on error
   */
  ssize_t rx(buffer_t::iterator buf_begin,
             buffer_t::iterator buf_end,
             const buffer_t::size_type position);

  /** \brief Push data to network (transmit)
   *
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
   *  \param [in] position Position transmitted block
   *  \return Processed size, -1 on error
   */
  ssize_t tx(buffer_t::iterator buf_begin,
             buffer_t::iterator buf_end,
             const buffer_t::size_type position);

  /**  Close all opened steams
   */
  void close();

  /** \brief Forward error to top level
   *
   *  Use property set_error_ as callback function, if property set
   *  \param [in] e_cod Error code
   *  \param [in] e_msg Error message
   */
  void set_error_if_first(const uint16_t e_cod, std::string_view e_msg) const;

  /** \brief Check directory exist
   *
   *  Common used method
   *  \param [in] chk_dir Path
   *  \return True if exist, or false
   */
  bool check_directory(std::string_view chk_dir) const;

  friend class session;
};

} // namespace tftp

#endif /* SOURCE_TFTP_DATA_MGR_H_ */
