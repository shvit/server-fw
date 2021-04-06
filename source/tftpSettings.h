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


#include "tftp_common.h"


namespace tftp
{

// -----------------------------------------------------------------------------

/**
 * \brief Settings storage class
 *
 *  Class for store server settings.
 *  Can't simple construct, create only from Settings::create() as shared pointer
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
  fLogMsg log_;  ///< External callback for logging message
};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSETTINGS_H_ */
