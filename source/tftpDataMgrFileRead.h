/*
 * tftpDataMgrFileRead.h
 *
 *  Created on: 3 авг. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPDATAMGRFILEREAD_H_
#define SOURCE_TFTPDATAMGRFILEREAD_H_

#include <fstream>
#include <experimental/filesystem>

#include "tftpCommon.h"
#include "tftpLogger.h"
#include "tftpDataMgrFile.h"

using namespace std::experimental;

namespace tftp
{

/// Alias for filesystem::path
using Path = filesystem::path;

using Perms = filesystem::perms;

//------------------------------------------------------------------------------

namespace ext
{

/** \brief Data manage streams for files
 */

class DataMgrFileRead: public DataMgrFile
{
protected:

  std::ifstream file_in_;  ///< Input file stream

public:

  /** \brief Default constructor
   *
   *  No need public construct. Construct allowed only from Session
   */
  DataMgrFileRead();

  //DataMgrFileRead(const DataMgrFileRead &) = delete; ///< Deleted/unused

  //DataMgrFileRead(DataMgrFileRead &) = delete; ///< Deleted/unused

  //DataMgrFileRead(DataMgrFileRead &&) = delete; ///< Deleted/unused

  //DataMgrFileRead & operator=(const DataMgrFileRead &) = delete; ///< Deleted/unused

  //DataMgrFileRead & operator=(DataMgrFileRead &) = delete; ///< Deleted/unused

  //DataMgrFileRead & operator=(DataMgrFileRead &&) = delete; ///< Deleted/unused

  /** Destructor
   */
  virtual ~DataMgrFileRead() override;

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


} // namespace ext

} // namespace tftp

#endif /* SOURCE_TFTPDATAMGRFILEREAD_H_ */
