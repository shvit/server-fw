/**
 * \file tftp_common.cpp
 * \brief Common TFTP server module
 *
 *  Base type/classes/functions
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <string>
#include <arpa/inet.h>
#include <numeric>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>

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

auto get_uid_by_name(const std::string & name) -> uid_t
{
  auto bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
  if(bufsize < 0) bufsize = 16384;

  struct passwd pwd;
  struct passwd *result;
  Buf buffer(bufsize, 0);

  getpwnam_r(
      name.c_str(),
      & pwd,
      buffer.data(),
      bufsize,
      &result);

  if(result == nullptr) return 0U;

  return pwd.pw_uid;
}

// -----------------------------------------------------------------------------

auto get_gid_by_name(const std::string & name) -> gid_t
{
  auto bufsize = sysconf(_SC_GETGR_R_SIZE_MAX);
  if(bufsize < 0) bufsize = 16384;

  struct group grp;
  struct group *result;
  Buf buffer(bufsize, 0);

  getgrnam_r(
      name.c_str(),
      & grp,
      buffer.data(),
      bufsize,
      &result);

  if(result == nullptr) return 0U;

  return grp.gr_gid;
}

// -----------------------------------------------------------------------------

} // namespace tftp

