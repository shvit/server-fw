/**
 * \file tftpAddr.h
 * \brief TFTP address buffer class module
 *
 *  License GPL-3.0
 *
 *  \date   22-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <arpa/inet.h>
#include <type_traits>
#include <string>
#include <regex>

#include "tftpAddr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

// Check buffer size for used socket addr
static_assert(
    (constants::max_sockaddr_size >= sizeof(struct sockaddr)) &&
    (constants::max_sockaddr_size >= sizeof(struct sockaddr_in)) &&
    (constants::max_sockaddr_size >= sizeof(struct sockaddr_in6)),
    "Fail buffer maximum size for sockaddr struct data!");

// -----------------------------------------------------------------------------

Addr::Addr():
    data_size_{0U}
{
  clear();
}

// -----------------------------------------------------------------------------

auto Addr::data_size() noexcept -> decltype(data_size_) &
{
  return data_size_;
}

// -----------------------------------------------------------------------------

auto Addr::data_size() const noexcept -> const decltype(data_size_) &
{
  return data_size_;
}

// -----------------------------------------------------------------------------

void Addr::clear()
{
  fill(0);
  data_size_ = 0U;
}

// -----------------------------------------------------------------------------

auto Addr::family() const noexcept -> const uint16_t &
{
  return *((sa_family_t *) data());
}

// -----------------------------------------------------------------------------

auto Addr::port() const -> uint16_t
{
  return be16toh(*((in_port_t *) (data()+sizeof(sa_family_t))));
}

// -----------------------------------------------------------------------------

auto Addr::as_sockaddr_ptr() noexcept -> struct sockaddr *
{
  return (struct sockaddr *) data();
}

// -----------------------------------------------------------------------------

void Addr::assign(const char * const & data_ptr, const size_t & new_size)
{
  if(data_ptr != nullptr)
  {
    data_size_ = std::min(size(), new_size);
    std::copy(data_ptr,
              data_ptr + data_size_,
              data());
  }
}

// -----------------------------------------------------------------------------

auto Addr::as_in() noexcept -> struct sockaddr_in &
{
  data_size_ = sizeof(struct sockaddr_in);
  return *((struct sockaddr_in *) data());
}

// -----------------------------------------------------------------------------

auto Addr::as_in6() noexcept -> struct sockaddr_in6 &
{
  data_size_ = sizeof(struct sockaddr_in6);
  return *((struct sockaddr_in6 *) data());
}

// -----------------------------------------------------------------------------

auto Addr::as_in() const noexcept -> const struct sockaddr_in &
{
  return *((struct sockaddr_in *) data());
}

// -----------------------------------------------------------------------------

auto Addr::as_in6() const noexcept -> const struct sockaddr_in6 &
{
  return *((struct sockaddr_in6 *) data());
}

// -----------------------------------------------------------------------------

auto Addr::str() const -> std::string
{
  std::array<char, 80> txt;

  switch(family())
  {
    case AF_INET:
      inet_ntop(
          family(),
          & as_in().sin_addr,
          txt.data(),
          txt.size());

      return std::string{txt.data()} + ":" + std::to_string(port());

    case AF_INET6:
      inet_ntop(
          family(),
          & as_in6().sin6_addr,
          txt.data(),
          txt.size());

      return "[" + std::string{txt.data()} + "]:" + std::to_string(port());

    default:
      return std::string{constants::unknown_addr};
  }
}

// -----------------------------------------------------------------------------

Addr::operator std::string() const
{
  return str();
}

// -----------------------------------------------------------------------------

void Addr::set_family(const uint16_t & new_family) noexcept
{
  *((uint16_t *) data()) = new_family;

  switch(new_family)
  {
    case AF_INET:
      data_size_ = sizeof(struct sockaddr_in);
      break;
    case AF_INET6:
      data_size_ = sizeof(struct sockaddr_in6);
      break;
    default:
      data_size_ = 0U;
      break;
  }
}

// -----------------------------------------------------------------------------

void Addr::set_port_u16(const uint16_t & new_port)
{
  *((uint16_t *) (data() + 2U)) = htobe16(new_port);
}

// -----------------------------------------------------------------------------

bool Addr::set_addr_str(const std::string & adr)
{
  bool ret=false;
  switch(family())
  {
    case AF_INET:
      ret = inet_pton(AF_INET, adr.c_str(), & as_in().sin_addr) > 0;
      break;
    case AF_INET6:
      ret = inet_pton(AF_INET6, adr.c_str(), & as_in6().sin6_addr) > 0;
      break;
    default:
      // Noithing to do
      break;
  }

  return ret;
}

// -----------------------------------------------------------------------------

bool Addr::set_port_str(const std::string & adr)
{
  bool ret = false;

  try
  {
    auto new_port = std::stoul(adr);
    new_port &= 0xFFFFUL;
    set_port((uint16_t) new_port);
    ret = true;
  } catch (...) { };

  return ret;
}

// -----------------------------------------------------------------------------

void Addr::set_addr_in(const in_addr & adr)
{
  set_family(AF_INET);

  std::copy((char *) & adr,
            ((char *) & adr) + sizeof(in_addr),
            (char *) & as_in().sin_addr);
}

// -----------------------------------------------------------------------------

void Addr::set_addr_in6(const in6_addr & adr)
{
  set_family(AF_INET6);

  std::copy((char *) & adr,
            ((char *) & adr) + sizeof(in6_addr),
            (char *) & as_in6().sin6_addr);
}

// -----------------------------------------------------------------------------

auto Addr::set_string(std::string_view new_value) -> std::tuple<bool,bool>
{
  bool is_set_addr = false;
  bool is_set_port = false;
  std::string tmp{new_value};
  std::regex  re4("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})$|(^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}):(\\d{0,5}))$|^:(\\d{1,5})$");
  std::regex  re6("^\\[?([0-9a-fA-F\\:]*)[\\]]?:(\\d{1,5})$|^\\[?([0-9a-fA-F\\:]*)[\\]]?$");
  std::smatch sm4, sm6;

  bool is_ipv4 = (std::regex_search(tmp, sm4, re4) && (sm4.size() == 6));
  bool is_ipv6 = std::regex_search(tmp, sm6, re6);

  if(is_ipv4)
  {
    set_family(AF_INET);

    // port
    if(std::string port_s{sm4[5].str()}; port_s.size())
    {
      is_set_port = set_port(port_s);
    }
    else
    if(std::string port_s{sm4[4].str()}; port_s.size())
    {
      is_set_port = set_port(port_s);
    }

    // addr
    if(std::string addr_s{sm4[3].str()}; addr_s.size())
    {
      is_set_addr = set_addr(addr_s);
    }
    else
    if(std::string addr_s{sm4[1].str()}; addr_s.size())
    {
      is_set_addr = set_addr(addr_s);
    }
  }
  else
  if(is_ipv6)
  {
    set_family(AF_INET6);

    // port
    if(std::string port_s{sm6[2].str()}; port_s.size())
    {
      is_set_port = set_port(port_s);
    }

    // addr
    if(std::string addr_s{sm6[3].str()}; addr_s.size())
    {
      is_set_addr = set_addr(addr_s);
    }
    else
    if(std::string addr_s{sm6[1].str()}; addr_s.size())
    {
      is_set_addr = set_addr(addr_s);
    }
  }

  return {is_set_addr, is_set_port};
;
}

// -----------------------------------------------------------------------------

} // namespace tftp
