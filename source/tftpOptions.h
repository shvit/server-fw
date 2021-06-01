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

  SrvReq      request_type_;    ///< Server request

  std::string filename_;        ///< Requested filename

  TransfMode  transfer_mode_;   ///< Transfer mode

  OptInt blksize_;

  OptInt timeout_;

  OptInt tsize_;

  OptInt windowsize_;

public:

  // Constructors, operators

  Options();

  Options(const Options &) = default;

  Options(Options &) = default;

  Options(Options &&) = default;

  virtual ~Options();

  Options & operator=(const Options &) = default;

  Options & operator=(Options &) = default;

  Options & operator=(Options &&) = default;

  // Getters

  auto blksize() const -> const int &;

  auto timeout() const -> const int &;

  auto tsize() const -> const int &;

  auto windowsize() const -> const int &;

  auto request_type() const -> const SrvReq &;

  auto filename() const -> const std::string &;

  auto transfer_mode() const -> const TransfMode &;

  bool was_set_blksize() const;

  bool was_set_timeout() const;

  bool was_set_tsize() const;

  bool was_set_windowsize() const;

  bool was_set_any() const;

  // Procesing methods

  bool buffer_parse(
      const SmBuf & buf,
      const size_t & buf_size,
      fLogMsg cb_logging);

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPOPTIONS_H_ */
