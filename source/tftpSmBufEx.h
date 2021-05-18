/*
 * tftpSmBufEx.h
 *
 *  Created on: 18 мая 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPSMBUFEX_H_
#define SOURCE_TFTPSMBUFEX_H_

#include "tftpSmBuf.h"

namespace tftp
{

//------------------------------------------------------------------------------

class SmBufEx: public SmBuf
{
protected:

  size_t data_size_;

public:

  template<typename ... Ts>
  SmBufEx(Ts && ... args);

  ~SmBufEx();

  auto data_size() const -> const size_t &;

  template<typename T>
  bool push_data(T && value);



};

//------------------------------------------------------------------------------

template<typename ... Ts>
SmBufEx::SmBufEx(Ts && ... args):
    SmBuf(std::forward<Ts>(args) ...),
    data_size_{0U}
{
}

//------------------------------------------------------------------------------

template<typename T>
bool SmBufEx::push_data(T && value)
{
  using TT = std::decay_t<T>;
  bool ret = false;

  if constexpr (std::is_integral_v<TT>)
  {
    if((data_size_ + sizeof(T)) <= size())
    {
      data_size_ += set_hton(data_size_, value);
      ret = true;
    }
  }
  else
  if constexpr (std::is_constructible_v<std::string, T>)
  {
    std::string tmp_str{std::forward<T>(value)};

    if((data_size_ + tmp_str.size()) <= size())
    {
      data_size_ += set_string(data_size_, tmp_str, true);
      ret = true;
    }
  }

  return ret;
}

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSMBUFEX_H_ */
