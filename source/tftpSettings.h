/**
 * \file tftpSettings.h
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

#include "tftpCommon.h"
#include "tftpAddr.h"


namespace tftp
{

using VecStr = std::vector<std::string>;

// -----------------------------------------------------------------------------

namespace constants
{
  constexpr uint16_t         default_tftp_port        = 69;
  constexpr uint16_t         default_retransmit_count = 3U;
  constexpr uint16_t         default_fb_dialect       = 3U;
  constexpr int              default_tftp_syslog_lvl  = 6;
  constexpr int              default_file_chmod       = 0664;
  constexpr std::string_view default_fb_lib_name      = "libfbclient.so";
}

// -----------------------------------------------------------------------------

/**
 * \brief Settings storage class
 *
 *  Class for store server settings.
 *  Can't simple construct - make it shareable
 *  Create only from Settings::create() as shared pointer
 */

class Settings: public std::enable_shared_from_this<Settings>
{
protected:

  /// No public constructor
  Settings();

public:

  bool        is_daemon;   ///< Flag showing run as daemon
  Addr        local_base_; ///< Listening server family/address/port
  std::string root_dir;    ///< Root directory of TFTP server
  VecStr      backup_dirs; ///< Search directories

  // firebird connect info
  std::string lib_dir;  ///< Directory with access library
  std::string lib_name; ///< Firebird access library filename
  std::string db;      ///< Database of firebird
  std::string user;    ///< User name access firebird
  std::string pass;    ///< Password access firebird
  std::string role;    ///< Role access firebird
  uint16_t    dialect; ///< Firebird server dialect

  //logger
  int use_syslog; ///< Syslog pass level logging message
  fLogMsg log_;   ///< External callback for logging message

  // protocol
  uint16_t retransmit_count_;

  std::string file_chown_user;
  std::string file_chown_grp;
  int         file_chmod;

  /** \brief Public creator
   *
   *  \return Shared pointer to this class
   */
  static auto create() -> pSettings;

  /** \brief Destructor
   */
  virtual ~Settings();

  /** \brief Load settings from CMD arguments
   *
   *  \param [in] argc Count of elements in argv
   *  \param [in] argv Array of arguments
   *  \return True if success, else - false
   */
  bool load_options(int argc, char * argv[]);

  void out_id(std::ostream & stream) const;

  void out_help(std::ostream & stream, std::string_view app) const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSETTINGS_H_ */
