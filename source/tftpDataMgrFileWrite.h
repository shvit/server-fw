/*
 * tftpDataMgrFileWrite.h
 *
 *  Created on: 3 авг. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPDATAMGRFILEWRITE_H_
#define SOURCE_TFTPDATAMGRFILEWRITE_H_


#include <fstream>

#include "tftpCommon.h"
#include "tftpLogger.h"
#include "tftpDataMgrFile.h"
#include "tftpFileNewAttr.h"

namespace tftp
{

//------------------------------------------------------------------------------

/** \brief Data manage streams for WRITE files
 */
class DataMgrFileWrite: public DataMgrFile
{
protected:

  std::ofstream fs_;  ///< Input file stream

  FileNewAttr  attr_; ///< File attributes for created files

  /** \brief Default constructor
   *
   *  No need public construct. Construct from create()
   */
  DataMgrFileWrite(
      fLogMsg logger,
      fSetError err_setter,
      std::string_view filename,
      std::string root_dir);

public:

  static auto create(
      fLogMsg logger,
      fSetError err_setter,
      std::string_view filename,
      std::string root_dir) -> pDataMgrFileWrite;


  /** Destructor
   */
  virtual ~DataMgrFileWrite() override;

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
  virtual bool init() override;

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
   *  Close stream
   *  Set owners and file mode if stream active
   */
  virtual void close() override;

  virtual void cancel() override;

};

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPDATAMGRFILEWRITE_H_ */
