/**
 * \file tftpSrvSettingsStor.h
 * \brief TFTP Settings class header
 *
 *  Storage class for TFTP settings
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPSETTINGS_H_
#define SOURCE_TFTPSETTINGS_H_

#include <shared_mutex>

#include "tftpCommon.h"
#include "tftpArgParser.h"
#include "tftpAddr.h"


namespace tftp
{

// -----------------------------------------------------------------------------

namespace constants
{
  //constexpr uint16_t         default_tftp_port        = 69;
  //constexpr uint16_t         default_retransmit_count = 3U;
  constexpr uint16_t         default_fb_dialect       = 3U;
  //constexpr int              default_tftp_syslog_lvl  = 6;
  constexpr int              default_file_chmod       = 0664;
  constexpr std::string_view default_fb_lib_name      = "libfbclient.so";


  /** \brief Settings for parsing CMD argument options
   *
   *  Used for parse with class ArgParser
   */
  const ArgItems srv_arg_settings =
  {
    {0, {}, ArgExistVaue::no, "", "Simple tftp firmware server 'server-fw' v"+std::string{constants::app_version}+" licensed GPL-3.0" , ""},
    {0, {}, ArgExistVaue::no, "", "Github project page https://github.com/shvit/server-fw", ""},
    {0, {}, ArgExistVaue::no, "", "(c) 2019-2021 Vitaliy.V.Shirinkin, e-mail: vitaliy.shirinkin@gmail.com", ""},
    {0, {}, ArgExistVaue::no, "", "--", ""},
    {0, {}, ArgExistVaue::no, "", "Some features:", ""},
    {0, {}, ArgExistVaue::no, "", "  - Recursive search requested files by md5 sum in search directory", ""},
    {0, {}, ArgExistVaue::no, "", "  - Use Firebird SQL server as file storage (optional requirement)", ""},
    {0, {}, ArgExistVaue::no, "", "Usage:", ""},
    {0, {}, ArgExistVaue::no, "", "./server-fw [<options ...>] {<IPv4>|[<IPv6>]}[:<UPD port>]", ""},
    {0, {}, ArgExistVaue::no, "", "Default listening is 0.0.0.0:"+std::to_string(constants::default_tftp_port), ""},
    {0, {}, ArgExistVaue::no, "", "(sample IPv4 \"192.168.0.1:69\", sample IPv6 \"[::1]:69\")", ""},
    {0, {}, ArgExistVaue::no, "", "Possible options:", ""},
    { 1, {"l", "L", "ip", "listen"}, ArgExistVaue::required, "IP[:port]", "Listening address and port", "deprecated"},
    { 2, {"h", "H", "help", "?"},    ArgExistVaue::no,       "",          "Show help information", ""},
    { 3, {"v", "V", "verb"},         ArgExistVaue::optional, "0..7",      "Set verbosity mode with logging level", "default 7 - debug"},
    { 3, {"s", "S", "syslog"},       ArgExistVaue::optional, "0..7",      "Set verbosity mode with logging level", "deprecated"},
    { 4, {"lib-dir"},                ArgExistVaue::required, "path",      "Directory for search library", "for DB client"},
    { 5, {"lib-name"},               ArgExistVaue::required, "filename",  "Library filename for DB client", "defailt "+std::string{constants::default_fb_lib_name}},
    { 6 ,{"root-dir"},               ArgExistVaue::required, "path",      "Root server directory", ""},
    { 7, {"search"},                 ArgExistVaue::required, "path",      "Directory for recursive search by md5 sum", "may be much"},
    { 8, {"fb-db"},                  ArgExistVaue::required, "database",  "Firebird access database name", ""},
    { 9, {"fb-user"},                ArgExistVaue::required, "username",  "Firebird access user name", ""},
    {10, {"fb-pass"},                ArgExistVaue::required, "password",  "Firebird access password", ""},
    {11, {"fb-role"},                ArgExistVaue::required, "role",      "Firebird access role", ""},
    {12, {"fb-dialect"},             ArgExistVaue::required, "1...3",     "Firebird server dialect (default "+std::to_string(constants::default_fb_dialect), ""},
    {13, {"daemon"},                 ArgExistVaue::no,       "",          "Run as daemon", ""},
    {14, {"retransmit"},             ArgExistVaue::required, "N",         "Maximum retransmit count if fail", "default "+std::to_string(constants::default_retransmit_count)},
    {15, {"file-chuser"},            ArgExistVaue::required, "username",  "Set user owner for created files", "default root"},
    {16, {"file-chgrp"},             ArgExistVaue::required, "group name","Set group owner for created files", "default root"},
    {17, {"file-chmod"},             ArgExistVaue::required, "permission","Set permissions for created files", "default 0664"},
  };

}

// -----------------------------------------------------------------------------

/**
 * \brief Server settings storage class
 *
 *  Class for store server settings.
 *  Can't simple construct - make it shareable
 *  Create only from SrvSettingsStor::create() as shared pointer
 */

class SrvSettingsStor: public std::enable_shared_from_this<SrvSettingsStor>
{
protected:

  mutable std::shared_mutex mutex_; ///< RW mutex for threading access

  ArgParser ap_; ///< Argument parser

  /// No public constructor
  SrvSettingsStor();

public:

  bool        is_daemon;   ///< Flag showing run as daemon
  Addr        local_addr; ///< Listening server family/address/port
  std::string root_dir;    ///< Root directory of TFTP server
  VecStr      search_dirs; ///< Secondary search directories (0..N)
  int         verb;           ///< Syslog pass level logging message
  uint16_t    retransmit_count_;
  std::string file_chown_user;
  std::string file_chown_grp;
  int         file_chmod;

  // firebird connect info
  std::string lib_dir;  ///< Directory with access library
  std::string lib_name; ///< Firebird access library filename
  std::string db;       ///< Database of firebird
  std::string user;     ///< User name access firebird
  std::string pass;     ///< Password access firebird
  std::string role;     ///< Role access firebird
  uint16_t    dialect;  ///< Firebird server dialect

  //logger

  /** \brief Public creator
   *
   *  \return Shared pointer to this class
   */
  static auto create() -> pSrvSettingsStor;

  /** \brief Destructor
   */
  virtual ~SrvSettingsStor();

  /** \brief Create shared locker for settings_
   *
   *  Use for direct read operations with settings_
   *  \return Lock object
   */
  auto begin_shared() const -> std::shared_lock<std::shared_mutex>;

  /** \brief Create unique locker for settings_
   *
   *  Use for direct write/read operations with settings_
   *  \return Lock object
   */
  auto begin_unique() const -> std::unique_lock<std::shared_mutex>;

  /** \brief Load settings from CMD arguments
   *
   *  \param [in] cb_logger Callback logger
   *  \param [in] argc Count of elements in argv
   *  \param [in] argv Array of arguments
   *  \return True if success, else - false
   */
  bool load_options(
      fLogMsg cb_logger,
      int argc,
      char * argv[]);

  void out_id(std::ostream & stream) const;

  void out_help(std::ostream & stream, std::string_view app) const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSETTINGS_H_ */
