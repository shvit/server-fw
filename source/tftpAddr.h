/**
 * \file tftpAddr.h
 * \brief TFTP address buffer class header
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
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
 *  Now support (easy use) only IPv4 and IPv6 families
 *  Other families need use raw access with: data(), size()
 *
 */
class Addr: public std::array<char, constants::max_sockaddr_size>
{
protected:

  /** \brief  Data size; optional for set real data size value
   *
   *  Sample use - for recvfrom() as "fromlen" pointer
   */
  socklen_t data_size_;

  /* \brief Set port value
   *
   *  \param [in] new_port New port value
   */
  void set_port_u16(const uint16_t & new_port);

  /* \brief Set port value from string value
   *
   *  \param [in] new_port New port value string
   *  \return True if convert was success, else - false
   */
  bool set_port_str(const std::string & adr);

  /* \brief Set address value from string value
   *
   *  Now support only IPv4 and IPv6
   *  Warning! Before use need set family value or use set_string()
   *  \param [in] adr New address value string
   *  \return True if convert was success, else - false
   */
  bool set_addr_str(const std::string & adr);

  /* \brief Set IPv4 address value
   *
   *  \param [in] adr New address value
   */
  void set_addr_in(const in_addr & adr);

  /* \brief Set IPv6 address value
   *
   *  \param [in] adr New address value
   */
  void set_addr_in6(const in6_addr & adr);

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

  /** \brief Set L4 port value
   *
   *  Value types: integer, string
   *  \param [in] new_port New port value (some type)
   *  \return True if success, else - false
   */
  template<typename T>
  bool set_port(const T & new_port);

  /** \brief Set L3 address value
   *
   *  Value types: in_addr, in6_addr, string
   *  Warning! Before use string need set family value or use set_string()
   *  \param [in] new_addr New address value (some type)
   *  \return True if success, else - false
   */
  template<typename T>
  bool set_addr(const T & new_addr);

  /** \brief Set address (and if exist - port) from string
   *
   *  Support formats:
   *  - 8.8.8.8 = only IPv4 adrress
   *  - 4.4.4.4:8080 = IPv4 address and port
   *  - fe80::1 = only IPv6 address
   *  - [fe80::1]:80 = IPv6 address and port
   *  \param [in] new_port New string value
   *  \return Two booleans {was set address, was set port} (true if success)
   */
  auto set_string(std::string_view new_value) -> std::tuple<bool,bool>;

  /** \brief Compare address only
   *
   *  Algoirithm: equal family and equal address bytes
   *  Port is ignore
   *  \param [in] right Second argumant
   *  \return True if addersses is equal, else - false
   */
  bool eqv_addr_only(const Addr & right);
};

// -----------------------------------------------------------------------------

template<typename T>
bool Addr::set_port(const T & new_port)
{
  bool ret = false;
  if constexpr (std::is_integral_v<T>)
  {
    set_port_u16((uint16_t) new_port);
    ret = true;
  }
  else
  if constexpr (std::is_constructible_v<std::string, T>)
  {
    std::string temp_str{new_port};
    ret = set_port_str(temp_str);
  }

  return ret;
}


// -----------------------------------------------------------------------------

template<typename T>
bool Addr::set_addr(const T & new_addr)
{
  bool ret = false;
  if constexpr (std::is_same_v<T, in_addr>)
  {
    set_addr_in(new_addr);
    ret = true;
  }
  else
  if constexpr (std::is_same_v<T, sockaddr_in>)
  {
    set_family(new_addr.sin_family);
    set_port(ntohs(new_addr.sin_port));
    set_addr_in(new_addr.sin_addr);
    ret = true;
  }
  else
  if constexpr (std::is_same_v<T, in6_addr>)
  {
    set_addr_in6(new_addr);
    ret = true;
  }
  else
  if constexpr (std::is_constructible_v<std::string, T>)
  {
    std::string temp_str{new_addr};
    ret = set_addr_str(temp_str);
  }

  return ret;
}

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPADDR_H_ */
