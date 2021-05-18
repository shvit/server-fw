/*
 * tftpSmBufEx.cpp
 *
 *  Created on: 18 мая 2021 г.
 *      Author: svv
 */

#include "tftpSmBufEx.h"

namespace tftp
{

//------------------------------------------------------------------------------

SmBufEx::~SmBufEx()
{
}

//------------------------------------------------------------------------------

auto SmBufEx::data_size() const -> const size_t &
{
  return data_size_;
}

//------------------------------------------------------------------------------

} // namespace tftp

