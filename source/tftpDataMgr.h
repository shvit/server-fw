/**
 * \file tftpDataMgr.h
 * \brief Data manager abstract class header
 *
 *  Base data manager class
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPIDATAMGR_H_
#define SOURCE_TFTPIDATAMGR_H_

#include "tftpCommon.h"
#include "tftpSrvBase.h"
#include "tftpSmBufEx.h"
#include "tftpLogger.h"

namespace tftp
{

// -----------------------------------------------------------------------------


/** \brief Data manage abstract class
 *
 * Class with file/database stream operations.
 * Create/close/read/write streams.
 * For use need override abstract methods
 */
class DataMgr
{
protected:

  // Processing info
  SrvReq        request_type_; ///< Request type
  size_t        file_size_;    ///< File size
  fSetError     set_error_;    ///< Callback error parsing to top level

  /** \brief Forward error to top level
   *
   *  Use property set_error_ as callback function, if property set
   *  \param [in] e_cod Error code
   *  \param [in] e_msg Error message
   */
  void set_error_if_first(
      const uint16_t e_cod,
      std::string_view e_msg) const;

  /** Check requested value is md5 sum
   *
   *  Match by regex used 'regex_template_md5'
   *  /return True if md5, else - false
   */
  bool match_md5(const std::string & val) const;

public:

  DataMgr();

  /** \brief Destructor
   */
  virtual ~DataMgr();

  /** Check active (opened stream) - abstract method
   */
  virtual bool active() const = 0;

  /** \brief Initialize streams - abstract method
   *
   *  \param [in] sett Settings of tftp server
   *  \param [in] cb_error Callback for error forward
   *  \param [in] opt Options of tftp protocol
   *  \return True on success, else - false
   */
  virtual bool init(
      SrvBase & sett,
      fSetError cb_error,
      const Options & opt) = 0;

  /** \brief Write data stream operations - abstract method
   *
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position received block (offset)
   *  \return Processed size, -1 on error
   */
  virtual auto write(
      SmBufEx::const_iterator buf_begin,
      SmBufEx::const_iterator buf_end,
      const size_t & position) -> ssize_t = 0;

  /** \brief Read data stream operations - abstract method
   *
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position transmitted block (offset)
   *  \return Processed size, -1 on error
   */
  virtual auto read(
      SmBufEx::iterator buf_begin,
      SmBufEx::iterator buf_end,
      const size_t & position) -> ssize_t = 0;

  /**  Close all opened steams - abstract method
   */
  virtual void close() = 0;

};

//##############################################################################

namespace ext
{

class DataMgr: public Logger
{
protected:

  // Processing info
  size_t        file_size_; ///< File size

  fSetError     set_error_; ///< Callback error parsing to top level

  /** \brief Forward error to top level
   *
   *  Use property set_error_ as callback function, if property set
   *  \param [in] e_cod Error code
   *  \param [in] e_msg Error message
   */
  void set_error_if_first(
      const uint16_t e_cod,
      std::string_view e_msg) const;

  /** Check requested value is md5 sum
   *
   *  Match by regex used 'regex_template_md5'
   *  /return True if md5, else - false
   */
  bool match_md5(std::string_view val) const;

public:

  DataMgr();

  /** \brief Destructor
   */
  virtual ~DataMgr();

  /** Check active (opened stream) - abstract method
   */
  virtual bool active() const = 0;

  /** \brief Initialize streams - abstract method
   *
   *  \param [in] sett Settings of tftp server
   *  \param [in] cb_error Callback for error forward
   *  \param [in] opt Options of tftp protocol
   *  \return True on success, else - false
   */
  virtual bool init(
      SrvBase & sett,
      fSetError cb_error,
      const Options & opt) = 0;

  /** \brief Write data stream operations - abstract method
   *
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position received block (offset)
   *  \return Processed size, -1 on error
   */
  virtual auto write(
      SmBufEx::const_iterator buf_begin,
      SmBufEx::const_iterator buf_end,
      const size_t & position) -> ssize_t = 0;

  /** \brief Read data stream operations - abstract method
   *
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position transmitted block (offset)
   *  \return Processed size, -1 on error
   */
  virtual auto read(
      SmBufEx::iterator buf_begin,
      SmBufEx::iterator buf_end,
      const size_t & position) -> ssize_t = 0;

  /**  Normal close all opened steams - abstract method
   */
  virtual void close() = 0;

  /**  Cancel all opened steams - abstract method
   *
   *  If data will write, then need delete it:
   *  - delete file for filesytem
   *  - rollback transaction for DB
   */
  virtual void cancel() = 0;

};

}
// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPIDATAMGR_H_ */
