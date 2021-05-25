/**
 * \file tftpDataMgrFile.h
 * \brief Data manager class for files header
 *
 *  Data manager for files
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
#include "tftpDataMgr.h"

using namespace std::experimental;

namespace tftp
{

/// Alias for filesystem::path
using Path = filesystem::path;

// -----------------------------------------------------------------------------

/** \brief Data manage streams for files
 */
class DataMgrFile: public DataMgr, public Base
{
protected:

  // Processing info
  std::ifstream file_in_;      ///< Input file stream
  std::ofstream file_out_;     ///< Output file stream

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
   *  No public construct.
   *  Construct allowed only from friend Session
   */
  DataMgrFile();

  DataMgrFile(DataMgrFile &&) = default;

  DataMgrFile & operator=(DataMgrFile &&) = default;

  /** Destructor
   */
  virtual ~DataMgrFile();

  /** Check active (opened streams or Firebird connection)
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
      pSettings & sett,
      fSetError cb_error,
      const Options & opt) override;

  /** \brief Pull data from network (receive)
   *
   *  Overrided virtual method for file streams
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
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
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
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

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_DATA_MGR_H_ */
