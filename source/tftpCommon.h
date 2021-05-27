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

#include <cxxabi.h> // for current class/type name
#include <functional>
#include <vector>
#include <memory>
#include <string>


namespace tftp
{

// -----------------------------------------------------------------------------

class Addr;

class Base;

class Srv;

class Session;

class DataMgr;

using pDataMgr = std::unique_ptr<DataMgr>;

class DataMgrFile;

class Settings;

using pSettings = std::shared_ptr<Settings>;

class Options;

using Buf = std::vector<char>;

class SmBuf;

class SmBufEx;

// -----------------------------------------------------------------------------

/** \bief Server request enum
 */
enum class SrvReq: uint16_t
{
  unknown=0, ///< Unknown request
  read=1,    ///< Read request
  write=2,   ///< Write request
};

// -----------------------------------------------------------------------------

/** \bief Server transfer mode enum
 *
 *  Notify: "mail" mode not supported
 */
enum class TransfMode: uint16_t
{
  unknown=0,  ///< Unknown transfer mode
  netascii=1, ///< Text transfer mode
  octet=2,    ///< Octet (binary) transfer mode
  binary,     ///< Binary transfer mode (fake mode, auto change to octet)
  mail,
};

// -----------------------------------------------------------------------------

/** \brief Logging level (for logging messages)
 */
enum class LogLvl: int
{
  emerg   = 0, // LOG_EMERG   // system is unusable
  alert   = 1, // LOG_ALERT   // action must be taken immediately
  crit    = 2, // LOG_CRIT    // critical conditions
  err     = 3, // LOG_ERR     // error conditions
  warning = 4, // LOG_WARNING // warning conditions
  notice  = 5, // LOG_NOTICE  // normal but significant condition
  info    = 6, // LOG_INFO    // informational
  debug   = 7, // LOG_DEBUG   // debug-level messages
};

// -----------------------------------------------------------------------------

/** \brief SessionState
 *
 */
enum class State: int
{
  need_init=0,
  error_and_stop,
  ack_options,
  data_tx,
  data_rx,
  ack_tx,
  ack_rx,
  retransmit,
  finish,
};

// -----------------------------------------------------------------------------

enum class TripleResult: int
{
  nop=0, // no operation - good state
  ok,    // ok processed - good state
  fail,  // fail state
};

// -----------------------------------------------------------------------------

/** \bief Callabck for custom logging message from server
 *
 *  \param [in] Logging level
 *  \param [in] Message text
 */
using fLogMsg = std::function<void(const LogLvl, std::string_view)>;

/** \brief Callback for function when set tftp error
 *
 *  \param [in] Error code
 *  \param [in] Error message text
 */
using fSetError = std::function<void(const uint16_t, std::string_view)>;

// -----------------------------------------------------------------------------

/** \brief Any common constants
 */
namespace constants
{

  /// Template for match MD5 by regex
  const std::string regex_template_md5{"([a-fA-F0-9]{32})"};

}

// -----------------------------------------------------------------------------

template<typename... Ts>
struct is_container_helper {};

template<typename T, typename _ = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container
<
  T,
  std::conditional_t
  <
    false,
    is_container_helper
    <
      typename T::value_type,
      typename std::enable_if_t
      <
        std::is_trivially_copyable_v
        <
          typename T::value_type> &&
          !std::is_pointer_v<typename T::value_type
        >,
        void
      >,
      decltype(std::declval<T>().cbegin()),
      decltype(std::declval<T>().cend())
    >,
    void
  >
>: public std::true_type {};

template<class T>
constexpr bool is_container_v = is_container<T>::value;


// -----------------------------------------------------------------------------

/** Get string with typename class T
 *  \return Typename
 */
template<typename T>
auto curr_type() -> std::string
{
  int stat{0};
  char * tmp_name = abi::__cxa_demangle(typeid(T).name(), 0, 0, & stat);
  if(!tmp_name) return "???";
  std::string ret{tmp_name};
  free(tmp_name);
  return ret;
}

// -----------------------------------------------------------------------------

#define CASE_OPER_TO_STR_VIEW(NAME) \
    case std::decay_t<decltype(val)>::NAME: return #NAME;

/** \brief Convert value of 'Request Type' to text string
 *
 *  \param [in] val Source argument
 *  \return String
 */
constexpr auto to_string(const SrvReq & val) -> std::string_view
{
  switch(val)
  {
    CASE_OPER_TO_STR_VIEW(unknown);
    CASE_OPER_TO_STR_VIEW(read);
    CASE_OPER_TO_STR_VIEW(write);
    default: return "UNK_SRV_REQ";
  }
}

/** \brief Convert value of 'Transfer Mode' to text string
 *
 *  \param [in] val Source argument
 *  \return String
 */
constexpr auto to_string(const TransfMode & val) -> std::string_view
{
  switch(val)
  {
    CASE_OPER_TO_STR_VIEW(unknown);
    CASE_OPER_TO_STR_VIEW(netascii);
    CASE_OPER_TO_STR_VIEW(octet);
    CASE_OPER_TO_STR_VIEW(binary);
    default: return "UNK_TRANSF_MODE";
  }
}

/** \brief Convert value of 'TLogging Level' to text string
 *
 *  \param [in] val Source argument
 *  \return String
 */
constexpr auto to_string(const LogLvl & val) -> std::string_view
{
  switch(val)
  {
    CASE_OPER_TO_STR_VIEW(emerg);
    CASE_OPER_TO_STR_VIEW(alert);
    CASE_OPER_TO_STR_VIEW(crit);
    CASE_OPER_TO_STR_VIEW(err);
    CASE_OPER_TO_STR_VIEW(warning);
    CASE_OPER_TO_STR_VIEW(notice);
    CASE_OPER_TO_STR_VIEW(info);
    CASE_OPER_TO_STR_VIEW(debug);
    default: return "UNK_LOG_LVL";
  }
}

constexpr auto to_string(const State & val) -> std::string_view
{
  switch(val)
  {
    CASE_OPER_TO_STR_VIEW(need_init);
    CASE_OPER_TO_STR_VIEW(error_and_stop);
    CASE_OPER_TO_STR_VIEW(ack_options);
    CASE_OPER_TO_STR_VIEW(data_tx);
    CASE_OPER_TO_STR_VIEW(data_rx);
    CASE_OPER_TO_STR_VIEW(ack_tx);
    CASE_OPER_TO_STR_VIEW(ack_rx);
    CASE_OPER_TO_STR_VIEW(retransmit);
    CASE_OPER_TO_STR_VIEW(finish);
    default: return "UNK_SESS_STATE";
  }
}

constexpr auto to_string(const TripleResult & val) -> std::string_view
{
  switch(val)
  {
    CASE_OPER_TO_STR_VIEW(nop);
    CASE_OPER_TO_STR_VIEW(ok);
    CASE_OPER_TO_STR_VIEW(fail);
    default: return "UNK_RES";
  }
}

#undef CASE_OPER_TO_STR_VIEW

// -----------------------------------------------------------------------------

/** \brief Operator+ for use in make logging messages
 *
 *  Need conversion function 'to_string(T)'
 *  \param [in] left Left string type operand
 *  \param [in] right Right enum type operand
 *  \return Result string
 */
template<typename T>
auto operator+(std::string_view left, const T & right)
//    -> std::enable_if_t<std::is_enum_v<T>, std::string>
    -> std::enable_if_t<
        !std::is_same_v<decltype(to_string(std::declval<T>())), void>,
        std::string>
{
  std::string ret{left};
  ret.append(to_string(right));
  return ret;
}

// -----------------------------------------------------------------------------

/** \brief Conversion: Socket address struct to string
 *
 *  Address can be as 'struct sockaddr_in', 'struct sockaddr_in6'
 *  \param [in] addr_begin Address buffer - begin iterator
 *  \param [in] addr_end Address buffer - end iterator
 *  \return String with L3 address and port info; sample "0.0.0.0:69"
 */
std::string sockaddr_to_str(
    Buf::const_iterator addr_begin,
    Buf::const_iterator addr_end);


// -----------------------------------------------------------------------------

/** \brief Conversion string to lowercase
 *
 *  \param [in,out] val String
 */
void do_lower(std::string & val);

// -----------------------------------------------------------------------------

/** \brief Check source string has digits only (0...9)
 *
 *  \param [in] val Source string
 *  \return True if string has digits only (len >= 1), else - false
 */
bool is_digit_str(std::string_view val);

// -----------------------------------------------------------------------------

/** \brief Get UID by user name
 *
 *  If fail, return 0U (root UID)
 *  \param [in] name User name
 *  \return UID
 */
auto get_uid_by_name(const std::string & name) -> uid_t;

/** \brief Get GID by group name
 *
 *  If fail, return 0U (root GID)
 *  \param [in] name Group name
 *  \return GID
 */
auto get_gid_by_name(const std::string & name) -> gid_t;

// -----------------------------------------------------------------------------

/** \brief Context (current function) with given messagess
 *
 *  Warning! Use only inside methods with base class tftp::Base
 *
 *  \param [in] MSG Text context with append message
 */
#define CURR_MSG(MSG) \
  curr_type<std::remove_pointer_t<decltype(this)>>()+"::"+__func__+"() "+MSG

/** \brief Logging with given level and full message
 *
 *  Notify: Logging without adding context!
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] LEVEL Logging messages level (from type LogLvl)
 *  \param [in] MSG Text message
 */
#define LOG_FMSG(LEVEL,FMSG) log(LogLvl::LEVEL,FMSG)

/** \brief Logging with context
 *
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] LEVEL Logging messages level (from type LogLvl)
 *  \param [in] MSG Text message
 */
#define LOG(LEVEL,MSG) LOG_FMSG(LEVEL,CURR_MSG(MSG))

/** \brief Logging with context as LOG_DEBUG message
 *
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] MSG Text message
 */
#define L_DBG(MSG) LOG(debug,   MSG)

/** \brief Logging with context as LOG_INFO message
 *
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] MSG Text message
 */
#define L_INF(MSG) LOG(info,    MSG)

/** \brief Logging with context as LOG_NOTICEg message
 *
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] MSG Text message
 */
#define L_NTC(MSG) LOG(notice,  MSG)

/** \brief Logging with context as LOG_WARNING message
 *
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] MSG Text message
 */
#define L_WRN(MSG) LOG(warning, MSG)

/** \brief Logging with context as LOG_ERR message
 *
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] MSG Text message
 */
#define L_ERR(MSG) LOG(err,     MSG)

/** \brief Logging with context and throw std::runtime_error
 *
 *  Used when error detected
 *  Warning! Use only inside methods with base class tftp::Base
 *  \param [in] MSG Text message
 */
#define ERROR_THROW_RUNTIME(MSG) \
    {\
      std::string full_msg{CURR_MSG(MSG)};\
      LOG_FMSG(err, full_msg);\
      throw std::runtime_error(full_msg);\
    };

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_COMMON_H_ */
