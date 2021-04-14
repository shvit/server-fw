/**
 * \file tftpSmBuf.cpp
 * \brief Smart buffer module
 *
 *  SmBuf class for network buffer manipulation
 *
 *  License GPL-3.0
 *
 *  \date   13-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include "tftpSmBuf.h"

namespace tftp
{

//------------------------------------------------------------------------------

void SmBuf::check_offset(
    std::string_view point,
    const size_t & offset,
    const size_t & t_size) const
{
  if((offset + t_size) > size())
  {
    throw std::invalid_argument(
        std::string{point}+": "+
        "Offset "+std::to_string(offset)+
        " with type size "+std::to_string(t_size)+
        " is over buffer size "+std::to_string(size()));
  }
}

//------------------------------------------------------------------------------

auto SmBuf::get_string(
    const size_t & offset,
    const size_t & buf_len) const -> std::string
{
  check_offset(__PRETTY_FUNCTION__, offset, buf_len);

  auto curr_beg_it = cbegin()+offset;
  auto curr_end_it = buf_len>0 ? cbegin()+offset+buf_len : cend();

  return std::string{curr_beg_it,
                     std::find(curr_beg_it, curr_end_it, 0)};
}

//------------------------------------------------------------------------------

auto SmBuf::set_string(
    const size_t & offset,
    std::string_view str,
    bool check_zero_end) -> ssize_t
{
  auto zero_it = (check_zero_end ?
      std::find(str.cbegin(),
                str.cend(),
                0) : str.cend());

  ssize_t new_size = std::distance(
      str.cbegin(),
      zero_it);

  assert(new_size >= 0); // never do it

  check_offset(
      __PRETTY_FUNCTION__,
      offset,
      new_size + (check_zero_end ? 1 : 0));

  std::copy(str.cbegin(), zero_it, begin() + offset);

  if(check_zero_end) (*this)[offset+new_size++] = 0; // zero end

  return new_size;
}

//------------------------------------------------------------------------------

} // namespace tftp

