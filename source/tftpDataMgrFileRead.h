/**
 * \file tftpDataMgrFileRead.h
 * \brief DataMgrFileRead class header
 *
 *  License GPL-3.0
 *
 *  \date 03-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPDATAMGRFILEREAD_H_
#define SOURCE_TFTPDATAMGRFILEREAD_H_

#include <fstream>

#include "tftpCommon.h"
#include "tftpLogger.h"
#include "tftpDataMgrFile.h"

namespace tftp
{

//------------------------------------------------------------------------------

/** \brief Data manage streams for READ files
 */
class DataMgrFileRead: public DataMgrFile
{
protected:

  std::ifstream fs_;  ///< Input file stream

  /** \brief Default constructor
   *
   *  No need public construct. Construct from create()
   */
   DataMgrFileRead(
      fLogMsg logger,
      fSetError err_setter,
      std::string_view filename,
      std::string_view root_dir,
      const VecStr & search_dirs);

public:

  static auto create(
      fLogMsg logger,
      fSetError err_setter,
      std::string_view filename,
      std::string_view root_dir,
      const VecStr & search_dirs) -> pDataMgrFileRead;


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
  virtual bool open() override;

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

  virtual void cancel() override;

};

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPDATAMGRFILEREAD_H_ */
