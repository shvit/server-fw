/**
 * \file tftpAddr.h
 * \brief TFTP address buffer class header
 *
 *  License GPL-3.0
 *
 *  \date   22-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTPADDR_H_
#define SOURCE_TFTPADDR_H_

#include <netinet/in.h> // sockaddr_in6
#include <array>
#include <stddef.h>

namespace tftp
{

// -----------------------------------------------------------------------------

namespace constants{

  /// Maximum buffer size for store address with any family
  constexpr size_t max_sockaddr_size = 30;

  /// String returned for wrong address conversion
  constexpr std::string_view unknown_addr = "???";

}

// -----------------------------------------------------------------------------

/** \brief Class for address buffer manipulation
 *
 */
class Addr: public std::array<char, constants::max_sockaddr_size>
{
protected:

  /** \brief  Data size; optional for set real data size value
   *
   *  Same type as socklen_t
   *  Sample use - for recvfrom() "fromlen" pointer
   */
  unsigned int data_size_;

public:

  /** \brief default constructor
   *
   *  Initialize buffer with 0.
   */
  Addr();

  /** \brief Get data size as reference (can change)
   *
   *  \return Refernce to value
   */
  auto data_size() noexcept -> decltype(data_size_) &;

  /** \brief Get data size (const version)
   *
   *  \return Data size
   */
  auto data_size() const noexcept -> const decltype(data_size_) &;

  /** \brief Clear buffer
   */
  void clear();

  /* \brief Get pointer buffer as sockaddr
   *
   *  Sample use - sendto()
   *  \return Pointer
   */
  auto as_sockaddr_ptr() noexcept -> struct sockaddr *;

  /* \brief Copy data from other buffer
   *
   *  Over size data ignored
   *  Set new value for "data_size"
   *  \param [in] data_ptr Buffer start pointer
   *  \param [in] new_size Size of data
   */
  void assign(
      const char * const & data_ptr,
      const size_t & new_size);

  /* \brief Data buffer as sockaddr_in reference (can change)
   *
   *  Set new value for "data_size"
   *  \return Reference
   */
  auto as_in() noexcept -> struct sockaddr_in &;

  /* \brief Data buffer as sockaddr_in (const version)
   *
   *  \return Const reference sockaddr_in
   */
  auto as_in() const noexcept -> const struct sockaddr_in &;

  /* \brief Data buffer as sockaddr_in6 reference (can change)
   *
   *  Set new value for "data_size"
   *  \return Reference
   */
  auto as_in6() noexcept -> struct sockaddr_in6 &;

  /* \brief Data buffer as sockaddr_in6 (const version)
   *
   *  \return Const reference sockaddr_in6
   */
  auto as_in6() const noexcept -> const struct sockaddr_in6 &;

  /** \brief Get family
   *
   *  \return Family value
   */
  auto family() const noexcept -> const uint16_t &;

  /** \brief Get port
   *
   *  Convert from network to host byte order
   *  \return Port value
   */
  auto port() const -> uint16_t;

  /** \brief Convert address buffer to string
   *
   *  \return String
   */
  auto str() const -> std::string;

  /** \brief Operator convert address buffer to string
   */
  operator std::string() const;

  /* \brief Set family value
   *
   *  Set new value for "data_size"
   *  \param [in] new_family New family value
   */
  void set_family(const uint16_t & new_family) noexcept;

  /* \brief Set port value
   *
   *  \param [in] new_port New port value
   */
  void set_port(const uint16_t & new_port);

  bool set_addr_str(const std::string & adr);

  bool set_port_str(const std::string & adr);

  void set_addr_in(const in_addr & adr);

  void set_addr_in6(const in6_addr & adr);

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPADDR_H_ */
