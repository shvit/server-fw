/**
 * \file tftpSmBufEx.cpp
 * \brief Smart buffer class SmBufEx module
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

#include "tftpSmBufEx.h"

namespace tftp
{

//------------------------------------------------------------------------------

SmBufEx::SmBufEx(
    const size_t & buf_size,
    bool is_int_be,
    bool is_str_zero):
        SmBuf(buf_size),
        data_size_{0U},
        val_int_bigendian_{is_int_be},
        val_str_zeroend_{is_str_zero}

{
}

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

void SmBufEx::clear()
{
  data_size_ = 0U;
}

//------------------------------------------------------------------------------

bool SmBufEx::is_bigendian() const
{
  return val_int_bigendian_;
}

//------------------------------------------------------------------------------

bool SmBufEx::is_littleendian() const
{
  return !val_int_bigendian_;
}

//------------------------------------------------------------------------------

bool SmBufEx::is_zeroend() const
{
  return val_str_zeroend_;
}

//------------------------------------------------------------------------------

void SmBufEx::set_bigendian()
{
  val_int_bigendian_ = true;
}

//------------------------------------------------------------------------------

void SmBufEx::set_littleendian()
{
  val_int_bigendian_ = false;
}

//------------------------------------------------------------------------------

void SmBufEx::set_zeroend()
{
  val_str_zeroend_ = true;
}

//------------------------------------------------------------------------------

void SmBufEx::set_not_zeroend()
{
  val_str_zeroend_ = false;
}

//------------------------------------------------------------------------------

} // namespace tftp

