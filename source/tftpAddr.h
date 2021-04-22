/*
 * tftpAddr.h
 *
 *  Created on: 22 апр. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPADDR_H_
#define SOURCE_TFTPADDR_H_

#include <array>
#include <stddef.h>

namespace tftp
{

// -----------------------------------------------------------------------------

namespace constants{

  constexpr size_t max_sockaddr_size = 30;

}

// -----------------------------------------------------------------------------

class Addr: public std::array<char, constants::max_sockaddr_size>
{
protected:
  size_t data_size_;

public:
  Addr();

  auto data_size() noexcept -> size_t &;

  void clear();

  auto as_sockaddr_ptr() noexcept -> struct sockaddr *;

  void assign(
      const char * const & data_ptr,
      const size_t & new_size);

  auto as_in() noexcept -> struct sockaddr_in &;

  auto as_in() const noexcept -> const struct sockaddr_in &;

  auto as_in6() noexcept -> struct sockaddr_in6 &;

  auto as_in6() const noexcept -> const struct sockaddr_in6 &;

  auto family() const noexcept -> const uint16_t &;

  auto port() const -> uint16_t;

  auto str() const -> std::string;

  operator std::string() const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPADDR_H_ */
