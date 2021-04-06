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
#include <ostream>
#include <string>
#include <syslog.h>
#include <vector>
#include <netinet/in.h>
#include <unistd.h>
#include <iostream>
#include <memory>


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

class Base;

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
}

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
