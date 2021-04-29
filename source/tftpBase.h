/**
 * \file tftpBase.h
 * \brief Base TFTP server class header
 *
 *  Base class for TFTP server
 *
 *  License GPL-3.0
 *
 *  \date   05-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTPBASE_H_
#define SOURCE_TFTPBASE_H_

#include <shared_mutex>

#include "tftpCommon.h"
#include "tftpSettings.h"

namespace tftp
{


// -----------------------------------------------------------------------------


/**
 * \brief Base class for child TFTP server classes
 *
 * Base class with server settings storage.
 * Thread safe access to storage by read/write mutex.
 * Base logging infrastructure.
 * Base settings setters and getters.
 * Method for parsing running arguments (options).
 *
 * External use settings can only with starting:
 * - begin_shared() for settings read operations
 * - begin_unique() for settings write operations
 */

class Base
{
protected:
  pSettings settings_;  ///< Shared pointer for settings storage

  mutable std::shared_mutex mutex_; ///< RW mutex for threading access

  /** \brief Create shared locker for settings_
   *
   *  Use for direct read operations with settings_
   */
  auto begin_shared() const -> std::shared_lock<std::shared_mutex>;

  /** \brief Create unique locker for settings_
   *
   *  Use for direct write/read operations with settings_
   */
  auto begin_unique() const -> std::unique_lock<std::shared_mutex>;

  auto local_base() -> Addr &;
  auto local_base() const -> const Addr &;

public:

  /** \brief Constructor
   *
   *  Use some default settings
   */
  Base();

  /** Copy constructor
   *
   */
  Base(const Base & src);

  /** Move constructor
   */
  Base(Base && src);

  /** Destructor
   */
  virtual ~Base();

  Base & operator=(Base && src);

  /** \brief Local used logger method
   *
   *  Use mutex shared mode
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  void log(LogLvl lvl, std::string_view msg) const;

  /** \brief Set: second custom logger
   *
   *  Use mutex unique mode
   *  \param [in] new_logger Custom function of logger
   */
  void set_logger(fLogMsg new_logger);

  /** \brief Set: syslog level
   *
   *  Use mutex unique mode
   *  \param [in] lvl Syslog level pass messages
   */
  //void set_syslog_level(const int lvl);

  /** \brief Get: syslog level
   *
   *  Use mutex shared mode
   *  \return Syslog level pass
   */
  //auto get_syslog_level_() const -> int;

  /** \brief Set: root server directory
   *
   *  Use mutex unique mode
   *  \param [in] root_dir Path to root server directory
   */
  //void set_root_dir(std::string_view root_dir);

  /** \brief Get: root server directory
   *
   *  Use mutex shared mode
   *  \return Path to root server directory
   */
  auto get_root_dir() const -> std::string;

  /** \brief Set: system library directory
   *
   *  Use for search library
   *  Default calculated; and default not used.
   *  Use mutex unique mode
   *  \param [in] dir Path to system library directory
   */
  //void set_lib_dir(std::string_view dir);

  /** \brief Get: system library directory
   *
   *  Use mutex shared mode
   *  \return Path to system library directory
   */
  auto get_lib_dir() const -> std::string;

  /** \brief Set: Firebird library name
   *
   *  Default set 'libfbclient.so' (const tftp::default_fb_name)
   *  Use mutex unique mode
   *  \param [in] fb_name Firebird library name
   */
  //void set_lib_name_fb(std::string_view fb_name);

  /** \brief Get: Firebird library name
   *
   *  Use mutex shared mode
   *  \return Name Firebird library name
   */
  auto get_lib_name_fb() const -> std::string;

  /** \brief Set: Firebird database name
   *
   *  Use mutex unique mode
   *  \param [in] val Firebird database name
   */
  //void set_connection_db(std::string_view val);

  /** \brief Set: Firebird access user name
   *
   *  Use mutex unique mode
   *  \param [in] val Firebird access user name
   */
  //void set_connection_user(std::string_view val);

  /** \brief Set: Firebird access password
   *
   *  Use mutex unique mode
   *  \param [in] val Firebird access password
   */
  //void set_connection_pass(std::string_view val);

  /** \brief Set: Firebird access role
   *
   *  \param [in] val Firebird access role
   *  Use mutex unique mode
   */
  //void set_connection_role(std::string_view val);

  /** \brief Set: Firebird access dialect
   *
   *  Use mutex unique mode
   *  \param [in] val Firebird access dialect
   */
  //void set_connection_dialect(uint16_t val);

  /** \brief Set: Firebird access information
   *
   *  Use mutex unique mode
   *  \param [in] db Firebird access database
   *  \param [in] user Firebird access username
   *  \param [in] pass Firebird access password
   *  \param [in] role Firebird access role
   *  \param [in] dialect Firebird access dialect
   */
  //void set_connection(std::string_view db,
  //                    std::string_view user,
  //                    std::string_view pass,
  //                    std::string_view role,
  //                    uint16_t         dialect);

  //void set_retransmit_count(const uint16_t & val);

  auto get_retransmit_count() const -> const uint16_t &;

  /** \brief Get: Firebird access information
   *
   *  Use mutex shared mode
   *  \return std::tuple {database, username, password, role, dialect}
   */
  auto get_connection() const
    -> std::tuple<std::string,
                  std::string,
                  std::string,
                  std::string,
                  uint16_t>;

  /** \brief Set: Flag is daemon
   *
   *  Use mutex unique mode
   *  \param [in] Flag is daemon
   */
  //void set_is_daemon(bool value);

  /** \brief Get: Flag is daemon
   *
   *  Use mutex shared mode
   *  \return Flag is daemon
   */
  bool get_is_daemon() const;

  /** \brief Set: Any/many search directories
   *
   *  Use mutex unique mode
   *  \param [in] args Search directories (0...Many)
   */
  //template<typename ... Ts>
  //void set_search_dir(Ts && ...);

  /** \brief Set: Append search directory
   *
   *  Use mutex unique mode
   *  \param [in] new_dir Search directory
   */
  //void set_search_dir_append(std::string_view new_dir);

  /** \brief Get searched directory
   *
   *  Use mutex shared mode
   *  \return Vector with strings
   */
  auto get_serach_dir() const -> std::vector<std::string>;

  /** \brief Set: Local base address and port from string
   *
   *  Use mutex unique mode
   *  \param [in] addr Address and port (addr:port) IPv4 or IPv6
   */
  //void set_local_base(std::string_view addr);

  /** \brief Set: Local base address and port as IPv4
   *
   *  Use mutex unique mode
   *  \param [in] addr Address IPv4
   *  \param [in] port Port
   */
  //void set_local_base_inet(struct in_addr * addr, uint16_t port);

  /** \brief Set: Local base address and port as IPv6
   *
   *  Use mutex unique mode
   *  \param [in] addr Address IPv6
   *  \param [in] port Port
   */
  //void set_local_base_inet6(struct in6_addr * addr, uint16_t port);

  /** \brief Get: Local base address and port as string
   *
   *  Use mutex shared mode
   *  \return Address and port
   */
  auto get_local_base_str() const -> std::string;

  /** \brief Fill all options from string set
   *
   *  Not use mutex, but sequentially use method which use mutex
   *  \param [in] argc Count of strings
   *  \param [in] argv Strings (char) array
   */
  bool load_options(int argc, char* argv[]);

  /** \brief Output to stream options help information
   *
   *  \param [in] stream Output stream
   *  \param [in] app Application name
   */
  void out_help(std::ostream & stream, std::string_view app) const;

  /** \brief Output to stream base id info
   *
   *  \param [out] stream Output stream
   */
  void out_id(std::ostream & stream) const;
};

//==============================================================================
/*
template<typename T>
auto Base::get_buf_item_raw(
    Buf & buf,
    const Buf::size_type offset) const
        -> std::enable_if_t<std::is_integral_v<T>, T &>
{
  if((offset + sizeof(T)) > buf.size())
  {
    std::invalid_argument("Offset "+std::to_string(offset)+
                          " is over buffer size");
  }

  return *((T *) (buf.data() + offset));
}

// -----------------------------------------------------------------------------

template<typename T>
auto Base::get_buf_item_ntoh(
    Buf & buf,
    const Buf::size_type offset) const
        -> std::enable_if_t<std::is_integral_v<T>, T>
{
  T value_n{get_buf_item_raw<T>(buf, offset)};
  std::reverse(((char *) & value_n),
               ((char *) & value_n) + sizeof(value_n));
  return value_n;
}

// -----------------------------------------------------------------------------

template<typename T>
auto Base::set_buf_item_raw(
    Buf & buf,
    const Buf::size_type offset,
    const T & value)
        -> std::enable_if_t<std::is_integral_v<T>, Buf::size_type>
{
  Buf::size_type ret_size = sizeof(T);
  if((offset + ret_size) > buf.size())
  {
    std::invalid_argument("Offset "+std::to_string(offset)+
                          " is over buffer size");
  }

  std::copy(((char *) & value),
            ((char *) & value) + sizeof(T),
            buf.data() + offset);

  return ret_size;
}

// -----------------------------------------------------------------------------

template<typename T>
auto Base::set_buf_item_hton(
    Buf & buf,
    const Buf::size_type offset,
    const T value)
        -> std::enable_if_t<std::is_integral_v<T>, Buf::size_type>
{
  T value_n{value};
  std::reverse(((char *) & value_n),
               ((char *) & value_n) + sizeof(value_n));
  return set_buf_item_raw<T>(buf, offset, value_n);
}

// -----------------------------------------------------------------------------
//template<typename ... Ts>
//void Base::set_search_dir(Ts && ... args)
//{
//  auto lk = begin_unique(); // write lock
//
//  settings_->backup_dirs.clear();
//
//  (settings_->backup_dirs.emplace_back(args), ...);
//}
// -----------------------------------------------------------------------------

template<typename T>
auto Base::set_buf_cont_str(
    Buf & buf,
    const Buf::size_type offset,
    const T & cntnr, bool check_zero_end)
        -> std::enable_if_t<is_container_v<T>, Buf::size_type>
{
  auto zero_it = (check_zero_end ?
      std::find(cntnr.cbegin(), cntnr.cend(), 0) : cntnr.cend());
  Buf::size_type new_size = std::distance(cntnr.cbegin(), zero_it);

  if(new_size)
  {
    if((offset + new_size + (check_zero_end?1:0)) > buf.size())
    {
      throw std::invalid_argument(
          "Offset "+std::to_string(offset)+
          " and new item size "+std::to_string(new_size)+
          " is over buffer size");
    }

    std::copy(cntnr.cbegin(), zero_it, buf.begin() + offset);
    if(check_zero_end) buf[offset+new_size++] = 0; // zero end
  }

  return new_size;
}

// -----------------------------------------------------------------------------
*/
} // namespace tftp

#endif /* SOURCE_TFTPBASE_H_ */
