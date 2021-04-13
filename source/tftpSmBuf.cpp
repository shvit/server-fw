/*
 * tftpSmBuf.cpp
 *
 *  Created on: 13 апр. 2021 г.
 *      Author: svv
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
        "Offset "+std::to_string(offset)+
        " with type size "+std::to_string(t_size)+
        " is over buffer size "+std::to_string(size()));
  }

}




//------------------------------------------------------------------------------

} // namespace tftp

