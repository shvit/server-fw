/**
 * \file tftp_common.h
 * \brief Common TFTP server header
 *
 *  Base classes for TFTP server
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTP_COMMON_H_
#define SOURCE_TFTP_COMMON_H_

#include <cassert>
#include <cxxabi.h> // for current class/type name
#include <functional>
#include <memory>
#include <ostream>
#include <shared_mutex>
#include <string>
#include <syslog.h>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>


#include <iostream>
#include <iomanip>


namespace tftp
{

// ----------------------------------------------------------------------------------

// constants
constexpr const in_port_t  default_tftp_port       = 69;
constexpr const int        default_tftp_syslog_lvl = 6;
constexpr const uint16_t   default_fb_dialect      = 3;
constexpr std::string_view default_fb_lib_name     = "libfbclient.so";

// ----------------------------------------------------------------------------------

class srv;

class session;

class data_mgr;

class settings_val;

using settings = std::shared_ptr<settings_val>;

using buffer_t = std::vector<char>;
using buffer_size_t = buffer_t::size_type;

template<typename T>
using option_t = std::tuple<bool, T>;

// ----------------------------------------------------------------------------------

/// Server request enum
enum class srv_req: uint16_t
{
  unknown=0, ///< Unknown request
  read=1,    ///< Read request
  write=2,   ///< Write request
};

// ----------------------------------------------------------------------------------

/// Server transfer mode enum
enum class transfer_mode: uint16_t
{
  unknown=0,  ///< Unknown transfer mode
  netascii=1, ///< Text transfer mode
  octet=2,    ///< Octet (binary) transfer mode
  binary,     ///< Binary transfer mode (fake mode, auto change to octet)
//mail,
};

// ----------------------------------------------------------------------------------

/// Callabck for custom logging message from server
using f_log_msg_t = std::function<void(const int, std::string_view)>;

// ----------------------------------------------------------------------------------

template<typename... Ts>
struct is_container_helper {};

template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<
        T,
        std::conditional_t<
            false,
            is_container_helper<
                typename T::value_type,
                typename std::enable_if_t<std::is_trivially_copyable_v<typename T::value_type> &&
                                          !std::is_pointer_v<typename T::value_type>,
                                          void>,
                decltype(std::declval<T>().cbegin()),
                decltype(std::declval<T>().cend())
                >,
            void
            >
        > : public std::true_type {};

template<class T>
constexpr bool is_container_v = is_container<T>::value;

// ----------------------------------------------------------------------------------

/**
 * \brief Settings storage class
 *
 *  Class for store server settings.
 *  Can't simple construct, create only from settings_val::create() as shared pointer
 */

class settings_val: public std::enable_shared_from_this<settings_val>
{
protected:

  /// No public constructor
  settings_val();

public:

  /// Public settings creator
  static std::shared_ptr<settings_val> create() { return std::make_shared<settings_val>(settings_val{}); };

  /// Destructor
  virtual ~settings_val() {};

  bool is_daemon; ///< Flag showing run as daemon

  buffer_t local_base_; ///< Listening server address:port (sockaddr_in*)

  // fb lib settings
  std::string lib_dir;  ///< System library directory (Ubuntu 18.04: /usr/lib/x86_64-linux-gnu/)
  std::string lib_name; ///< Firebird library filename (libfbclient.so)

  // storage directory
  std::string root_dir;                 ///< Root directory of simple TFTP server
  std::vector<std::string> backup_dirs; ///< Search directories of simple TFTP server

  // storage firebird
  std::string db;      ///< Database of firebird
  std::string user;    ///< User name access firebird
  std::string pass;    ///< Password access firebird
  std::string role;    ///< Role access firebird
  uint16_t    dialect; ///< Firebird server dialect

  //logger
  int use_syslog;    ///< Syslog pass level logging message
  f_log_msg_t log_;  ///< External callback for logging message
};

// ----------------------------------------------------------------------------------

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

class base
{
protected:
  settings settings_;  ///< Shared pointer for settings storage

  mutable std::shared_mutex mutex_; ///< Read/write mutex for settings threading access

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

  /** \brief Reference to settings_->local_base as struct sockaddr_in (IPv4)
   *
   *  Check size settings_->local_base and allocate if need.
   *  Not use mutex.
   */
  auto local_base_as_inet()  -> struct sockaddr_in &;

  /** \brief Reference to settings_->local_base as struct sockaddr_in6 (IPv6)
   *
   *  Check size settings_->local_base and allocate if need.
   *  Not use mutex.
   */
  auto local_base_as_inet6() -> struct sockaddr_in6 &;

  /** \brief Get raw value from buffer (as reference!)
   *
   *  \param [in] buf Operational buffer
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Referecne value type of T
   */
  template<typename T>
  auto get_buf_item_raw(buffer_t & buf, const buffer_size_t offset) const
    -> std::enable_if_t<std::is_integral_v<T>, T &>;

  /** \brief Get value from buffer and convert from network byte order to host byte order
   *
   *  \param [in] buf Operational buffer
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Value type of T
   */
  template<typename T>
  auto get_buf_item_ntoh(buffer_t & buf, const buffer_size_t offset) const
    -> std::enable_if_t<std::is_integral_v<T>, T>;

  /** \brief Set (write) raw value to buffer
   *
   *  \param [in,out] buf Operational buffer
   *  \param [in] offset Buffer offset (position) in bytes
   *  \return Size of buffer increment (size added value)
   */
  template<typename T>
  auto set_buf_item_raw(buffer_t & buf, const buffer_size_t offset, const T & value)
    -> std::enable_if_t<std::is_integral_v<T>, buffer_size_t>;

  /** \brief Set (write) raw value to buffer and convert from host byte order to network byte order
   *
   *  \param [in,out] buf Operational buffer
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] value Value for buffer add
   *  \return Size of buffer increment (size added value)
   */
  template<typename T>
  auto set_buf_item_hton(buffer_t & buf, const buffer_size_t offset, const T value)
    -> std::enable_if_t<std::is_integral_v<T>, buffer_size_t>;

  /** \brief Write container data to buffer
   *
   *  Can use for string with smart add zero end.
   *  \param [in,out] buf Operational buffer
   *  \param [in] offset Buffer offset (position) in bytes
   *  \param [in] cntnr Container data
   *  \param [in] check_zero_end Enable/disable smart end zero
   *  \return Size of buffer increment (size of added data)
   *
   */
  template<typename T>
  auto set_buf_cont_str(buffer_t & buf, const buffer_size_t offset, const T & cntnr, bool check_zero_end = false)
    -> std::enable_if_t<is_container_v<T>, buffer_size_t>;

public:

  /** \brief Constructor
   *
   *  Use some default settings
   */
  base();

  /** Copy constructor
   *
   */
  base(base & src);

  /** Move constructor
   */
  base(base && src);

  /** Destructor
   */
  virtual ~base();

  /** \brief Local used logger method
   *
   *  Use mutex shared mode
   *  \param [in] lvl Level of message (0-7)
   *  \param [in] msg Text message
   */
  void log(int lvl, std::string_view msg) const;

  /** \brief Set: second custom logger
   *
   *  Use mutex unique mode
   *  \param [in] new_logger Custom function of logger
   */
  void set_logger(f_log_msg_t new_logger);

  /** \brief Set: syslog level
   *
   *  Use mutex unique mode
   *  \param [in] lvl Syslog level pass messages
   */
  void set_syslog_level(const int lvl);

  /** \brief Get: syslog level
   *
   *  Use mutex shared mode
   *  \return Syslog level pass
   */
  auto get_syslog_level() const -> int;

  /** \brief Set: root server directory
   *
   *  Use mutex unique mode
   *  \param [in] root_dir Path to root server directory
   */
  void set_root_dir(std::string_view root_dir);

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
  void set_lib_dir(std::string_view dir);

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
  void set_lib_name_fb(std::string_view fb_name);

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
  void set_connection_db(std::string_view val);

  /** \brief Set: Firebird access user name
   *
   *  Use mutex unique mode
   *  \param [in] val Firebird access user name
   */
  void set_connection_user(std::string_view val);

  /** \brief Set: Firebird access password
   *
   *  Use mutex unique mode
   *  \param [in] val Firebird access password
   */
  void set_connection_pass(std::string_view val);

  /** \brief Set: Firebird access role
   *
   *  \param [in] val Firebird access role
   *  Use mutex unique mode
   */
  void set_connection_role(std::string_view val);

  /** \brief Set: Firebird access dialect
   *
   *  Use mutex unique mode
   *  \param [in] val Firebird access dialect
   */
  void set_connection_dialect(uint16_t val);

  /** \brief Set: Firebird access information
   *
   *  Use mutex unique mode
   *  \param [in] db Firebird access database
   *  \param [in] user Firebird access username
   *  \param [in] pass Firebird access password
   *  \param [in] role Firebird access role
   *  \param [in] dialect Firebird access dialect
   */
  void set_connection(std::string_view db,
                      std::string_view user,
                      std::string_view pass,
                      std::string_view role,
                      uint16_t         dialect);

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
  void set_is_daemon(bool value);

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
  template<typename ... Ts>
  void set_search_dir(Ts && ...);

  /** \brief Set: Append search directory
   *
   *  Use mutex unique mode
   *  \param [in] new_dir Search directory
   */
  void set_search_dir_append(std::string_view new_dir);

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
  void set_local_base(std::string_view addr);

  /** \brief Set: Local base address and port as IPv4
   *
   *  Use mutex unique mode
   *  \param [in] addr Address IPv4
   *  \param [in] port Port
   */
  void set_local_base_inet(struct in_addr * addr, uint16_t port);

  /** \brief Set: Local base address and port as IPv6
   *
   *  Use mutex unique mode
   *  \param [in] addr Address IPv6
   *  \param [in] port Port
   */
  void set_local_base_inet6(struct in6_addr * addr, uint16_t port);

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

//=================================================================================================================================

template<typename T>
auto base::get_buf_item_raw(buffer_t & buf, const buffer_size_t offset) const
  -> std::enable_if_t<std::is_integral_v<T>, T &>
{
  if((offset + sizeof(T)) > buf.size()) std::invalid_argument("Offset "+std::to_string(offset)+" is over buffer size");

  return *((T *) (buf.data() + offset));
};

// ----------------------------------------------------------------------------------

template<typename T>
auto base::get_buf_item_ntoh(buffer_t & buf, const buffer_size_t offset) const
  -> std::enable_if_t<std::is_integral_v<T>, T>
{
  T value_n{get_buf_item_raw<T>(buf, offset)};
  std::reverse(((char *) & value_n),
               ((char *) & value_n) + sizeof(value_n));
  return value_n;
};

// ----------------------------------------------------------------------------------

template<typename T>
auto base::set_buf_item_raw(buffer_t & buf, const buffer_size_t offset, const T & value)
  -> std::enable_if_t<std::is_integral_v<T>, buffer_size_t>
{
  buffer_size_t ret_size = sizeof(T);
  if((offset + ret_size) > buf.size()) std::invalid_argument("Offset "+std::to_string(offset)+" is over buffer size");

  std::copy(((char *) & value),
            ((char *) & value) + sizeof(T),
            buf.data() + offset);

  return ret_size;
};

// ----------------------------------------------------------------------------------

template<typename T>
auto base::set_buf_item_hton(buffer_t & buf, const buffer_size_t offset, const T value)
  -> std::enable_if_t<std::is_integral_v<T>, buffer_size_t>
{
  T value_n{value};
  std::reverse(((char *) & value_n),
               ((char *) & value_n) + sizeof(value_n));
  return set_buf_item_raw<T>(buf, offset, value_n);
};

// ----------------------------------------------------------------------------------

template<typename ... Ts>
void base::set_search_dir(Ts && ... args)
{
  auto lk = begin_unique(); // write lock

  settings_->backup_dirs.clear();

  (settings_->backup_dirs.emplace_back(args), ...);
};

// ----------------------------------------------------------------------------------

template<typename T>
auto base::set_buf_cont_str(buffer_t & buf, const buffer_size_t offset, const T & cntnr, bool check_zero_end)
  -> std::enable_if_t<is_container_v<T>, buffer_size_t>
{
  auto zero_it = (check_zero_end ? std::find(cntnr.cbegin(), cntnr.cend(), 0) : cntnr.cend());
  buffer_size_t new_size = std::distance(cntnr.cbegin(), zero_it);

  if(new_size)
  {
    if((offset + new_size + (check_zero_end?1:0)) > buf.size()) throw std::invalid_argument("Offset "+std::to_string(offset)+" and new item size "+std::to_string(new_size)+" is over buffer size");

    std::copy(cntnr.cbegin(), zero_it, buf.begin() + offset);
    if(check_zero_end) buf[offset+new_size++] = 0; // zero end
  }

  return new_size;
};

//=================================================================================================================================

/** Get string with typename class T
 *  \return Typename
 */
template<typename T>
auto curr_type() -> std::string
{
  int stat{0};
  char * tmp_name = abi::__cxa_demangle(typeid(T).name(), 0, 0, & stat);
  std::string ret{tmp_name};
  if(tmp_name) free(tmp_name);
  return ret;
};

//=================================================================================================================================

std::string_view to_string(const srv_req);       ///< Conversion 'srv_req' to text
std::string_view to_string(const transfer_mode); ///< Conversion 'transfer_mode' to text
std::string_view to_string_lvl(const int);       ///< Conversion debug level to text

// ----------------------------------------------------------------------------------

/** \brief Conversion: Socket address struct to string
 *
 *  Address can be as 'struct sockaddr_in', 'struct sockaddr_in6'
 *  \param [in] addr_begin Address buffer - begin iterator
 *  \param [in] addr_end Address buffer - end iterator
 *  \return String with L3 address and port info; sample "0.0.0.0:69"
 */
std::string sockaddr_to_str(buffer_t::const_iterator addr_begin,
                            buffer_t::const_iterator addr_end);

// ----------------------------------------------------------------------------------

/// Preprocessor defines for simple logging
#define THIS_CLASS_METHOD() curr_type<std::remove_pointer_t<decltype(this)>>().append("::").append(__func__).append("()")
#define LOG(LEVEL,MSG) log(LEVEL, THIS_CLASS_METHOD() + " " + MSG);

/// Preprocessor defines for runtime error (if bug snuck)
#define ERROR_CLASS_METHOD__RUNTIME(msg) \
    {\
      std::string full_msg{curr_type<std::remove_pointer_t<decltype(this)>>()};\
      full_msg.append("::").append(__PRETTY_FUNCTION__).append(": ").append(msg);\
      this->log(LOG_ERR, full_msg);\
      throw std::runtime_error(full_msg);\
    };

} // namespace tftp

#endif /* SOURCE_TFTP_COMMON_H_ */
