/**
 * \file tftpOptions.h
 * \brief TFTP Options class header
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPOPTIONS_H_
#define SOURCE_TFTPOPTIONS_H_

#include <tuple>

#include "tftpCommon.h"
#include "tftpSmBuf.h"

namespace tftp
{

// -----------------------------------------------------------------------------

/** \brief Constants for tftp options
 */
namespace constants
{
  // Default values for options
  constexpr int dflt_blksize    = 512;
  constexpr int dflt_timeout    = 10;
  constexpr int dflt_tsize      = 0; // zero - don't control size
  constexpr int dflt_windowsize = 1;

  // Default names for options
  constexpr std::string_view name_blksize      = "blksize";
  constexpr std::string_view name_timeout      = "timeout";
  constexpr std::string_view name_tsize        = "tsize";
  constexpr std::string_view name_windowsize   = "windowsize";
}

// -----------------------------------------------------------------------------

/** \bief Optional class storage with default value
 *
 *  Reamrk: std::optional<T> does't have default value
 */
using OptInt = std::tuple<bool, int>;

// -----------------------------------------------------------------------------

class Options
{
protected:

  SrvReq      request_type_;    ///< Server request type

  std::string filename_;        ///< Requested filename

  TransfMode  transfer_mode_;   ///< Transfer mode

  OptInt blksize_; ///< TFTP option 'blksize'

  OptInt timeout_; ///< TFTP option 'timeout'

  OptInt tsize_; ///< TFTP option 'tsize'

  OptInt windowsize_; ///< TFTP option 'windowsize'

public:

  // Constructors, operators

  /** \brief Default constructor
   */
  Options();

  /** \brief Destructor
   */
  virtual ~Options();

  // Default copy/move ---------------------------------------------------------

  Options(const Options &) = default;

  Options(Options &) = default;

  Options(Options &&) = default;

  Options & operator=(const Options &) = default;

  Options & operator=(Options &) = default;

  Options & operator=(Options &&) = default;

  // Getters -------------------------------------------------------------------

  /** \brief Get value option 'blksize'
   *
   *  \return Value as const reference
   */
  auto blksize() const -> const int &;

  /** \brief Get value option 'timeout'
   *
   *  \return Value as const reference
   */
  auto timeout() const -> const int &;

  /** \brief Get value option 'tsize'
   *
   *  \return Value as const reference
   */
  auto tsize() const -> const int &;

  /** \brief Get value option 'windowsize'
   *
   *  \return Value as const reference
   */
  auto windowsize() const -> const int &;

  /** \brief Get value request type
   *
   *  \return Value as const reference
   */
  auto request_type() const -> const SrvReq &;

  /** \brief Get value filename
   *
   *  \return Value as const reference
   */
  auto filename() const -> const std::string &;

  /** \brief Get value transfer mode
   *
   *  \return Value as const reference
   */
  auto transfer_mode() const -> const TransfMode &;

  /** \brief Flag value 'blksize' was changed
   *
   *  \return True if was change, else - false
   */
  bool was_set_blksize() const;

  /** \brief Flag value 'timeout' was changed
   *
   *  \return True if was change, else - false
   */
  bool was_set_timeout() const;

  /** \brief Flag value 'tsize' was changed
   *
   *  \return True if was change, else - false
   */
  bool was_set_tsize() const;

  /** \brief Flag value 'windowsize' was changed
   *
   *  \return True if was change, else - false
   */
  bool was_set_windowsize() const;

  /** \brief Flag any option value was changed
   *
   *  Check only: blksize, timeout, tsize, windowsize
   *  \return True if any was change, else - false
   */
  bool was_set_any() const;

  // Procesing methods ---------------------------------------------------------

  /** \brief Fill options values from buffer
   *
   *  Used for parse received TFTP packet
   *  \param [in] buf Buffer
   *  \param [in] buf_size Buffer size
   *  \param [in] cb_logging Callback for logging
   *  \return True if no fatal errors, else - false
   */
  bool buffer_parse(
      const SmBuf & buf,
      const size_t & buf_size,
      fLogMsg cb_logging);

  /** \brief Set 'blksize' from string value
   *
   *  \param [in] val String value
   *  \param [in] log Callbacl for logging; default is nullptr
   */
  void set_blksize(
      const std::string & val,
      fLogMsg log = nullptr);

  /** \brief Set 'timeout' from string value
   *
   *  \param [in] val String value
   *  \param [in] log Callbacl for logging; default is nullptr
   */
  void set_timeout(
      const std::string & val,
      fLogMsg log = nullptr);

  /** \brief Set 'windowsize' from string value
   *
   *  \param [in] val String value
   *  \param [in] log Callbacl for logging; default is nullptr
   */
  void set_windowsize(
      const std::string & val,
      fLogMsg log = nullptr);

  /** \brief Set 'tsize' from string value
   *
   *  \param [in] val String value
   *  \param [in] log Callbacl for logging; default is nullptr
   */
  void set_tsize(
      const std::string & val,
      fLogMsg log = nullptr);

  /** \brief Set transfer mode from string value
   *
   *  \param [in] val String value
   *  \param [in] log Callbacl for logging; default is nullptr
   *  \return True if no error, else - false
   */
  bool set_transfer_mode(
      const std::string & val,
      fLogMsg log = nullptr);

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPOPTIONS_H_ */
