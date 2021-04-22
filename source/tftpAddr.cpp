/*
 * tftpAddr.cpp
 *
 *  Created on: 22 апр. 2021 г.
 *      Author: svv
 */

#include <netinet/in.h> // sockaddr_in6
#include <vector>
#include <arpa/inet.h>

#include "tftpAddr.h"

namespace tftp
{

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
}

// -----------------------------------------------------------------------------

auto Addr::data_size() noexcept -> size_t &
{
  return data_size_;
}

// -----------------------------------------------------------------------------

void Addr::clear()
{
  fill(0);
}

// -----------------------------------------------------------------------------

auto Addr::family() const noexcept -> const uint16_t &
{
  return *((sa_family_t *) data());
}

// -----------------------------------------------------------------------------

auto Addr::port() const -> uint16_t
{
  return be16toh(*((in_port_t *) data()+sizeof(sa_family_t)));
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
  return *((struct sockaddr_in *) data());
}

// -----------------------------------------------------------------------------

auto Addr::as_in6() noexcept -> struct sockaddr_in6 &
{
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
      return "???";
  }
}

// -----------------------------------------------------------------------------

Addr::operator std::string() const
{
  return str();
}

// -----------------------------------------------------------------------------


} // namespace tftp
