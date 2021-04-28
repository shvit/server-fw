/**
 * \file tftp_common.cpp
 * \brief Common TFTP server module
 *
 *  Base type/classes/functions
 *
 *  License GPL-3.0
 *
 *  \date   06-nov-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <string>
#include <arpa/inet.h>
#include <numeric>

#include "tftpCommon.h"

namespace tftp
{

//------------------------------------------------------------------------------

std::string sockaddr_to_str(
    Buf::const_iterator addr_begin,
    Buf::const_iterator addr_end)
{
  using SA4 = struct sockaddr_in;
  using SA6 = struct sockaddr_in6;

  const auto addr_dist{std::distance(addr_begin, addr_end)};

  if(addr_dist < 2) return "";

  Buf txt(80, 0);

  auto & curr_family=*((uint16_t *) & *addr_begin);

  switch(curr_family)
  {
    case AF_INET:
      if((size_t)addr_dist < sizeof(SA4)) return "";

      inet_ntop(
          curr_family,
          & ((SA4 *) & *addr_begin)->sin_addr,
          txt.data(),
          txt.size());

      return std::string{txt.data()}+":"+
             std::to_string(be16toh(((SA4*) & *addr_begin)->sin_port));

    case AF_INET6:
      if((size_t)addr_dist < sizeof(SA6)) return "";

      inet_ntop(
          curr_family,
          & ((SA6 *) & *addr_begin)->sin6_addr,
          txt.data(),
          txt.size());

      return "[" + std::string{txt.data()}+"]:"+
             std::to_string(be16toh(((SA6*) & *addr_begin)->sin6_port));

    default:
      return "";
  }
}

// -----------------------------------------------------------------------------

void do_lower(std::string & val)
{
  std::transform(val.cbegin(),
                 val.cend(),
                 val.begin(),
                 [](const char v){ return (char)std::tolower(v); });
}

// -----------------------------------------------------------------------------

bool is_digit_str(std::string_view val)
{
  if(val.size() == 0U) return false;

  return std::accumulate(
      val.cbegin(),
      val.cend(),
      true,
      [](bool l, auto & c){ return l && std::isdigit(c); });
}

// -----------------------------------------------------------------------------

} // namespace tftp

