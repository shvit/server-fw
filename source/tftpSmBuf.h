/*
 * tftpSmBuf.h
 *
 *  Created on: 13 апр. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPSMBUF_H_
#define SOURCE_TFTPSMBUF_H_

#include <cassert>
#include <vector>

#include "tftpCommon.h"

namespace tftp
{

//------------------------------------------------------------------------------

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

  /** \brief Get value from buffer and convert network -> host byte order
   *
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Value type of T
   */
  template<typename T>
  auto get_ntoh(const size_t & offset) const
      -> std::enable_if_t<std::is_integral_v<T>, T>;

  /** \brief Set value integer type to buffer and convert to network byte order
   *
   *  Warning: need hard control of type T
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] val Value any integer type
   *  \return Data size passed to buffer
   */
  template<typename T>
  auto set_hton(const size_t & offset, const T & val)
      -> std::enable_if_t<std::is_integral_v<T>, ssize_t>;


  template<typename T>
  auto set_string(
      const size_t & offset,
      const T & str,
      bool check_zero_end = false)
          -> std::enable_if_t<is_container_v<T>, ssize_t>;

  using Buf::Buf;
};

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
auto SmBuf::get_ntoh(const size_t & offset) const
    -> std::enable_if_t<std::is_integral_v<T>, T>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  T tmp_val;

  std::copy((SmBuf::const_reverse_iterator)(cbegin()+offset+sizeof(T)),
            (SmBuf::const_reverse_iterator)(cbegin()+offset),
            (char *) & tmp_val);

  return tmp_val;

}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::set_hton(const size_t & offset, const T & val)
    -> std::enable_if_t<std::is_integral_v<T>, ssize_t>
{
  check_offset_type<T>(__PRETTY_FUNCTION__, offset);

  constexpr ssize_t t_size = (ssize_t)sizeof(T);

  std::copy(((char *) & val),
            ((char *) & val) + t_size,
            (SmBuf::reverse_iterator)(begin()+offset+t_size));

  return t_size;
}

//------------------------------------------------------------------------------

template<typename T>
auto SmBuf::set_string(
    const size_t & offset,
    const T & str,
    bool check_zero_end)
        -> std::enable_if_t<is_container_v<T>, ssize_t>
{
  auto zero_it = (check_zero_end ?
      std::find(str.cbegin(),
                str.cend(),
                0) : str.cend());

  ssize_t new_size = sizeof(T::value_type) * std::distance(
      str.cbegin(),
      zero_it);

  assert(new_size >= 0); // never do it

  check_offset_type(
      __PRETTY_FUNCTION__,
      offset,
      new_size + (check_zero_end ? 1 : 0));

  std::copy(str.cbegin(), zero_it, str.begin() + offset);

  if(check_zero_end) str[offset+new_size++] = 0; // zero end

  return new_size;
}

//------------------------------------------------------------------------------


} // namespace tftp

#endif /* SOURCE_TFTPSMBUF_H_ */
