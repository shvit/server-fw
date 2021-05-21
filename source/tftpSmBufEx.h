/**
 * \file tftpSmBufEx.h
 * \brief Smart buffer class SmBufEx header
 *
 *  SmBufEx class for extended network buffer manipulation
 *
 *  License GPL-3.0
 *
 *  \date   18-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTPSMBUFEX_H_
#define SOURCE_TFTPSMBUFEX_H_

#include "tftpSmBuf.h"

namespace tftp
{

//------------------------------------------------------------------------------

namespace constants
{
  constexpr bool default_buf_int_bigendian = true;
  constexpr bool default_buf_str_zeroend = true;
}

//------------------------------------------------------------------------------

/** \brief Smart buffer with push data methods
 *
 *  Now support only integer values, boolean, char and string values
 *  Calculates pushed data size (actual data size)
 *  Can write integer as big endian (default) or little endian
 *  Can write string with zero end (default) or without it
 */
class SmBufEx: public SmBuf
{
protected:

  size_t data_size_; ///< Pushed to buffer data size

  bool val_int_bigendian_; ///< Flag for buffer value as big endian

  bool val_str_zeroend_; ///< Flag for add zero at end of string

  /** \brief Push single value data to buffer
   *
   *  Now support only integer value and string value
   *  \param [in] val Value for write
   *  \return True if success pushed value, else false
   */
  template<typename T>
  bool push_item(T && val);

public:

  /** \brief No default constructor (need buffer size!)
   */
  SmBufEx() = delete;

  /** \brief Constructor with buffer size and flags: BE, zeroend
   *
   *  \param [in] buf_size Buffer size to allocate
   *  \param [in] is_int_be Flag BE
   *  \param [in] is_str_zero Flag zero end string
   */
  SmBufEx(
      const size_t & buf_size,
      bool is_int_be = constants::default_buf_int_bigendian,
      bool is_str_zero = constants::default_buf_str_zeroend);

  /** \brief Destructor
   */
  virtual ~SmBufEx();

  /** \brief get pushed data size (actual data size)
   *
   *  \return data size
   */
  auto data_size() const -> const size_t &;

  /** \brief Clear data (only reset data size, no init buffer)
   */
  void clear();

  /** \brief Push data to buffer
   *
   *  Now support only integer values and string values
   *  \param [in] args Variadic values to push buffer
   *  \return True if success pushed value, else false
   */
  template<typename ... Ts>
  bool push_data(Ts && ... args);

  /** \brief Check flag is "big endian"
   *
   *  \return Value of flag
   */
  bool is_bigendian() const;

  /** \brief Check flag is "little endian"
   *
   *  \return Value of flag
   */
  bool is_littleendian() const;

  /** \brief Check flag is "zero end string"
   *
   *  \return Value of flag
   */
  bool is_zeroend() const;

  /** \brief Set flag to "big endian"
   */
  void set_bigendian();

  /** \brief Set flag to "little endian"
   */
  void set_littleendian();

  /** \brief Set flag to "zero end string"
   */
  void set_zeroend();

  /** \brief Set flag to "not zero at end string"
   */
  void set_not_zeroend();

  void data_size_reset(size_t new_size);
};

//------------------------------------------------------------------------------

template<typename T>
bool SmBufEx::push_item(T && val)
{
  bool ret = false;
  using TT = std::decay_t<T>;

  if constexpr (std::is_integral_v<TT>)
  {
    if((data_size_ + sizeof(TT)) <= size())
    {
      ssize_t len = is_bigendian() ?
          set_be(data_size_, std::forward<T>(val)):
          set_le(data_size_, std::forward<T>(val));
      if((ret = (len >= 0)))
      {
        data_size_ += (size_t) len;
      }
    }
  }
  else
  if constexpr (std::is_constructible_v<std::string, T>)
  {
    std::string tmp_str{std::forward<T>(val)};

    if((data_size_ + tmp_str.size()) <= size())
    {
      ssize_t len=set_string(data_size_, tmp_str, val_str_zeroend_);
      if((ret = (len >= 0)))
      {
        data_size_ += (size_t) len;
      }
    }
  }

  return ret;
}

//------------------------------------------------------------------------------

template<typename ... Ts>
bool SmBufEx::push_data(Ts && ... args)
{
  bool ret = true;

  ((ret = push_item(std::forward<Ts>(args)) && ret), ...);

  return ret;
}

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSMBUFEX_H_ */
