/**
 * \file tftpSmBuf.h
 * \brief Smart buffer header
 *
 *  SmBuf class for network buffer manipulation
 *
 *  License GPL-3.0
 *
 *  \date 13-sep-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2.1
 */

#ifndef SOURCE_TFTPSMBUF_H_
#define SOURCE_TFTPSMBUF_H_

#include <cassert>
#include <vector>

#include "tftpCommon.h"

namespace tftp
{

//------------------------------------------------------------------------------

/** \brief Smart buffer with bufer manipulation get/set values
 *
 *  Support get/set intergral type (hton/nton/raw) and string
 *  Need manipulate with offset (start data position)
 */
class SmBuf: public Buf
{
protected:

  /** \brief Check offset + data size with buffer size
   *
   *  If check wrong throw exception "invalid_argument"
   *  \param [in] point Place of action (__PRETTY_FUNCTION)
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] t_size Data size
   */
  void check_offset(
      std::string_view point,
      const size_t & offset,
      const size_t & t_size) const;

  /** \brief Check offset + data sizeof(T) with buffer size
   *
   *  If check wrong throw exception "invalid_argument"
   *  \param [in] point Place of action (__PRETTY_FUNCTION)
   *  \param [in] offset Buffer offset (position) in bytes
   */
  template<typename T>
  void check_offset_type(
      std::string_view point,
      const size_t & offset) const;

public:

  virtual ~SmBuf();

  /** \brief Check offset and data size with buffer size
   *
   *  Result false if offset out of buffer (with data length is 0)
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] t_size Data size
   *  \return True if offset and data size valid, else - false
   */
  bool is_valid(const size_t & offset,
                const size_t & t_size) const noexcept;

  /** \brief Check offset and data size type T with buffer size
   *
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return True if offset and data size valid, else - false
   */
  template<typename T>
  bool is_valid(const size_t & offset) const noexcept;

  /** \brief Get raw value from buffer of given type
   *
   *  Warning! Repeat, offset in bytes
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Reference to value type of T &
   */
  template<typename T>
  auto raw(const size_t & offset)
      -> std::enable_if_t<std::is_integral_v<T>, T &>;

  /** \brief Get const raw value from buffer of given type
   *
   *  Warning! Repeat, offset in bytes
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Reference to value type of const T &
   */
  template<typename T>
  auto raw(const size_t & offset) const
      -> std::enable_if_t<std::is_integral_v<T>, const T &>;

  /** \brief Get big endian value from buffer as host byte order
   *
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Value type of T
   */
  template<typename T>
  auto get_be(const size_t & offset) const
      -> std::enable_if_t<std::is_integral_v<T>, T>;

  /** \brief Get little endian value from buffer as host byte order
   *
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Value type of T
   */
  template<typename T>
  auto get_le(const size_t & offset) const
      -> std::enable_if_t<std::is_integral_v<T>, T>;

  /** \brief Set value integer type to buffer as big endian
   *
   *  Warning: need hard control of type T
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] val Value any integer type
   *  \return Data size passed to buffer
   */
  template<typename T>
  auto set_be(const size_t & offset, const T & val)
      -> std::enable_if_t<std::is_integral_v<T>, ssize_t>;

  /** \brief Set value integer type to buffer as little endian
   *
   *  Warning: need hard control of type T
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] val Value any integer type
   *  \return Data size passed to buffer
   */
  template<typename T>
  auto set_le(const size_t & offset, const T & val)
      -> std::enable_if_t<std::is_integral_v<T>, ssize_t>;

  /** \brief Get string value from buffer
   *
   *  If buffer has 0, then string will strip from zero poition
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] buf_len Length of string (maximum)
   *  \return String value
   */
  auto get_string(
      const size_t & offset,
      const size_t & buf_len = 0) const -> std::string;

  /** \brief Write string/container data to buffer
   *
   *  Can use for string with smart add zero end.
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] srt String or container data
   *  \param [in] check_zero_end Enable/disable smart end zero
   *  \return Data size passed to buffer
   *
   */
  auto set_string(
      const size_t & offset,
      std::string_view str,
      bool check_zero_end = false) -> ssize_t;

  /** \brief Compare string with data in buffer
   *
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] srt String for compare data
   *  \param [in] check_zero_end Check zero at the end of data in buffer
   *  \return True if data string equal buffer
   *
   */
  bool eqv_string(
      const size_t & offset,
      std::string_view str,
      bool check_zero_end = false) const;


  using Buf::Buf;
};

//------------------------------------------------------------------------------

template<typename T>
bool SmBuf::is_valid(const size_t & offset) const noexcept
{
  return is_valid(offset, sizeof(T));
}

//------------------------------------------------------------------------------

template<typename T>
void SmBuf::check_offset_type(
    std::string_view point,
    const size_t & offset) const
{
  check_offset(point, offset, sizeof(T));
}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::raw(const size_t & offset)
    -> std::enable_if_t<std::is_integral_v<T>, T &>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  return *((T *) (data() + offset));

}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::raw(const size_t & offset) const
    -> std::enable_if_t<std::is_integral_v<T>, const T &>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  return *((T *) (data() + offset));

}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::get_be(const size_t & offset) const
    -> std::enable_if_t<std::is_integral_v<T>, T>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  switch(sizeof(T))
  {
    case 1U: return (T)*(cbegin()+offset);
    case 2U: return (T)be16toh(*((uint16_t*)(data()+offset)));
    case 4U: return (T)be32toh(*((uint32_t*)(data()+offset)));
    case 8U: return (T)be64toh(*((uint64_t*)(data()+offset)));
    default:
      throw std::invalid_argument(
          "Wrong integer size ("+std::to_string(sizeof(T))+")");
  }
}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::get_le(const size_t & offset) const
    -> std::enable_if_t<std::is_integral_v<T>, T>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  switch(sizeof(T))
  {
    case 1U: return (T)*(data()+offset);
    case 2U: return (T)le16toh(*((uint16_t*)(data()+offset)));
    case 4U: return (T)le32toh(*((uint32_t*)(data()+offset)));
    case 8U: return (T)le64toh(*((uint64_t*)(data()+offset)));
    default:
      throw std::invalid_argument(
          "Wrong integer size ("+std::to_string(sizeof(T))+")");
  }
}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::set_be(const size_t & offset, const T & val)
    -> std::enable_if_t<std::is_integral_v<T>, ssize_t>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  constexpr ssize_t t_size = (ssize_t)sizeof(T);

  switch(t_size)
  {
    case 1U:
      *(data()+offset) = val;
      break;
    case 2U:
      *((uint16_t*)(data()+offset)) = htobe16((uint16_t)val);
      break;
    case 4U:
      *((uint32_t*)(data()+offset)) = htobe32((uint32_t)val);
      break;
    case 8U:
      *((uint64_t*)(data()+offset)) = htobe64((uint64_t)val);
      break;
    default:
      throw std::invalid_argument(
          "Wrong integer size ("+std::to_string(t_size)+")");
  }

  return t_size;
}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::set_le(const size_t & offset, const T & val)
    -> std::enable_if_t<std::is_integral_v<T>, ssize_t>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  constexpr ssize_t t_size = (ssize_t)sizeof(T);

  switch(t_size)
  {
    case 1U:
      *(data()+offset) = val;
      break;
    case 2U:
      *((uint16_t*)(data()+offset)) = htole16((uint16_t)val);
      break;
    case 4U:
      *((uint32_t*)(data()+offset)) = htole32((uint32_t)val);
      break;
    case 8U:
      *((uint64_t*)(data()+offset)) = htole64((uint64_t)val);
      break;
    default:
      throw std::invalid_argument(
          "Wrong integer size ("+std::to_string(t_size)+")");
  }

  return t_size;
}

//------------------------------------------------------------------------------


} // namespace tftp

#endif /* SOURCE_TFTPSMBUF_H_ */
