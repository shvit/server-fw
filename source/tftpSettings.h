/*
 * tftpSettings.h
 *
 *  Created on: 6 апр. 2021 г.
 *      Author: svv
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
 *  Can't simple construct, create only from settings_val::create() as shared pointer
 */

class settings_val: public std::enable_shared_from_this<settings_val>
{
protected:

  /// No public constructor
  settings_val();

public:

  /// Public settings creator
  static std::shared_ptr<settings_val> create()
  {
    return std::make_shared<settings_val>(settings_val{});
  };

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

// -----------------------------------------------------------------------------




} // namespace tftp

#endif /* SOURCE_TFTPSETTINGS_H_ */
