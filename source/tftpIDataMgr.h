/*
 * tftpIDataMgr.h
 *
 *  Created on: 24 мая 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPIDATAMGR_H_
#define SOURCE_TFTPIDATAMGR_H_

#include "tftpCommon.h"
#include "tftpBase.h"

#include "tftpSmBufEx.h"

namespace tftp
{

// -----------------------------------------------------------------------------

namespace constants
{
  /// Template for match MD5 by regex
  const std::string regex_template_md5{"([a-fA-F0-9]{32})"};
}

// -----------------------------------------------------------------------------


/** \brief Data manage abstract class
 *
 * Class with file/database stream operations.
 * Create/close/read/write streams.
 * For use need override abstract methods
 */
class IDataMgr
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

  IDataMgr();

  /** \brief Destructor
   */
  virtual ~IDataMgr();

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
      pSettings & sett,
      fSetError cb_error,
      const Options & opt) = 0;

  /** \brief Write data stream operations - abstract method
   *
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
   *  \param [in] position Position received block
   *  \return Processed size, -1 on error
   */
  virtual auto write(
      SmBufEx::const_iterator buf_begin,
      SmBufEx::const_iterator buf_end,
      const size_t & position) -> ssize_t = 0;

  /** \brief Read data stream operations - abstract method
   *
   *  \param [in] buf_begin Block buffer - begin iterator
   *  \param [in] buf_end Block buffer - end iterator
   *  \param [in] position Position transmitted block
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

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPIDATAMGR_H_ */
