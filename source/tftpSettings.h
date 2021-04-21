/**
 * \file tftpSettings.h
 * \brief TFTP Settings class header
 *
 *  Storage class for TFTP settings
 *
 *  License GPL-3.0
 *
 *  \date   06-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTPSETTINGS_H_
#define SOURCE_TFTPSETTINGS_H_


#include "tftpCommon.h"


namespace tftp
{

// -----------------------------------------------------------------------------

namespace constants
{
  constexpr uint16_t default_tftp_port        = 69;
  constexpr uint16_t default_retransmit_count = 3U;
  constexpr uint16_t default_fb_dialect       = 3U;
  constexpr int      default_tftp_syslog_lvl  = 6;

  constexpr std::string_view default_fb_lib_name = "libfbclient.so";
}

// -----------------------------------------------------------------------------

/**
 * \brief Settings storage class
 *
 *  Class for store server settings.
 *  Can't simple construct
 *  Create only from Settings::create() as shared pointer
 */

class Settings: public std::enable_shared_from_this<Settings>
{
protected:

  /// No public constructor
  Settings();

public:

  /// Public creator
  static auto create() -> pSettings;

  /// Destructor
  virtual ~Settings();

  bool is_daemon; ///< Flag showing run as daemon

  Buf local_base_; ///< Listening server address:port (sockaddr_in*)

  // fb lib settings
  std::string lib_dir;  ///< System library directory
  std::string lib_name; ///< Firebird library filename

  // storage directory
  std::string root_dir;                 ///< Root directory of TFTP server
  std::vector<std::string> backup_dirs; ///< Search directories

  // storage firebird
  std::string db;      ///< Database of firebird
  std::string user;    ///< User name access firebird
  std::string pass;    ///< Password access firebird
  std::string role;    ///< Role access firebird
  uint16_t    dialect; ///< Firebird server dialect

  //logger
  int use_syslog;    ///< Syslog pass level logging message
  fLogMsg log_;  ///< External callback for logging message

  // protocol
  uint16_t retransmit_count_;
};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSETTINGS_H_ */
