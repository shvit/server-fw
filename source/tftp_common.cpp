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

#include <iostream>
#include <string>
#include <string_view>
#include <arpa/inet.h>

#include "tftp_common.h"

namespace tftp
{

//=================================================================================================================================

settings_val::settings_val():
  is_daemon{false},
  local_base_{},
  lib_dir{},
  lib_name{},
  root_dir{},
  backup_dirs{},
  db{},
  user{},
  pass{},
  role{},
  dialect{},
  use_syslog{},
  log_{nullptr}
{
}

//=================================================================================================================================


//=================================================================================================================================

#define CASE_OPER_TO_STR_VIEW(NAME) case decltype(val)::NAME: return #NAME;

std::string_view to_string(const srv_req val)
{
  switch(val)
  {
    CASE_OPER_TO_STR_VIEW(unknown);
    CASE_OPER_TO_STR_VIEW(read);
    CASE_OPER_TO_STR_VIEW(write);
    default:
      throw std::runtime_error("Unknown value");
  }
}

std::string_view to_string(const transfer_mode val)
{
  switch(val)
  {
    CASE_OPER_TO_STR_VIEW(unknown);
    CASE_OPER_TO_STR_VIEW(netascii);
    CASE_OPER_TO_STR_VIEW(octet);
    CASE_OPER_TO_STR_VIEW(binary);
    default:
      throw std::runtime_error("Unknown value");
  }
}

#undef CASE_OPER_TO_STR_VIEW

std::string_view to_string_lvl(const int val)
{
  switch(val)
  {
    case LOG_ALERT:   return "ALERT";
    case LOG_CRIT:    return "CRIT";
    case LOG_DEBUG:   return "DEBUG";
    case LOG_EMERG:   return "EMERG";
    case LOG_ERR:     return "ERROR";
    case LOG_INFO:    return "INFO";
    case LOG_NOTICE:  return "NOTOCE";
    case LOG_WARNING: return "WARNING";
    default:          return "UNKNOWN";
  }
}

//=================================================================================================================================

std::string sockaddr_to_str(buffer_t::const_iterator addr_begin,
                                   buffer_t::const_iterator addr_end)
{
  const auto addr_dist{std::distance(addr_begin, addr_end)};

  if(addr_dist < 2) return "";

  buffer_t txt(80, 0);

  auto curr_family=*((uint16_t *) & *addr_begin);

  switch(curr_family)
  {
    case AF_INET:
      if((size_t)addr_dist < sizeof(sockaddr_in)) return "";

      inet_ntop(curr_family, & ((sockaddr_in *) & *addr_begin)->sin_addr, txt.data(), txt.size());
      return std::string{txt.data()}+":"+std::to_string(be16toh(((struct sockaddr_in *) & *addr_begin)->sin_port));
    case AF_INET6:
      if((size_t)addr_dist < sizeof(sockaddr_in6)) return "";

      inet_ntop(curr_family, & ((sockaddr_in6 *) & *addr_begin)->sin6_addr, txt.data(), txt.size());
      return "[" + std::string{txt.data()}+"]:"+std::to_string(be16toh(((struct sockaddr_in6 *) & *addr_begin)->sin6_port));
    default:
      return "";
  }
}

// ----------------------------------------------------------------------------------

} // namespace tftp

