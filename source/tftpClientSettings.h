/**
 * \file  tftpClientSettingss.h
 * \brief Class ClientSettings header
 *
 *  Class for arguments parsing for TFTP client
 *
 *  License GPL-3.0
 *
 *  \date 09-jun-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPCLIENTSETTINGS_H_
#define SOURCE_TFTPCLIENTSETTINGS_H_

#include "tftpCommon.h"
#include "tftpAddr.h"
#include "tftpOptions.h"
#include "tftpArgParser.h"

namespace tftp
{

//------------------------------------------------------------------------------

namespace constants
{

  /** \brief Settings for parsing CMD argument options
   *
   *  Used for parse with class ArgParser
   */
  const ArgItems arg_option_settings =
  {
    {0, {}, tftp::ArgExistVaue::no, "", "Simple TFTP client from 'server-fw' project licensed GPL-3.0", ""},
    {0, {}, tftp::ArgExistVaue::no, "", "Github project page https://github.com/shvit/server-fw", ""},
    {0, {}, tftp::ArgExistVaue::no, "", "(c) 2019-2021 Vitaliy.V.Shirinkin, e-mail: vitaliy.shirinkin@gmail.com", ""},
    {0, {}, tftp::ArgExistVaue::no, "", "--", ""},
    {0, {}, tftp::ArgExistVaue::no, "", "Usage:", ""},
    {0, {}, tftp::ArgExistVaue::no, "", "./tftp-cl [<options ...>]", "<IP addr>[:<UDP Port>]"},
    {0, {}, tftp::ArgExistVaue::no, "", "Possible options:", ""},
    { 1, {"l", "L", "local"},      tftp::ArgExistVaue::required, "file", "Local file path and name", ""},
    { 2, {"r", "R", "remote"},     tftp::ArgExistVaue::required, "file", "Remote file name", ""},
    { 3, {"g", "G", "get"},        tftp::ArgExistVaue::no,       "",     "Get file from server", ""},
    { 4, {"p", "P", "put"},        tftp::ArgExistVaue::no,       "",     "Put file to server", ""},
    { 5, {"h", "H", "help", "?"},  tftp::ArgExistVaue::no,       "",     "Show help information", ""},
    { 6, {"v", "V", "verb"},       tftp::ArgExistVaue::optional, "level", "Set verbosity mode with logging level", "default 7 debug"},
    { 7, {"m", "M", "mode"},       tftp::ArgExistVaue::required, "mode", "TFTP transfer mode", ""},
    { 8, {"b", "B", "blksize"},    tftp::ArgExistVaue::required, "N",    "TFTP option 'block size'", "default 512"},
    { 9, {"t", "T", "timeout"},    tftp::ArgExistVaue::required, "N",    "TFTP option 'timeout'", "default 10"},
    {10, {"w", "W", "windowsize"}, tftp::ArgExistVaue::required, "N",    "TFTP option 'windowsize'", "default 1"},
    {11, {"Z", "z", "tsize"},      tftp::ArgExistVaue::optional, "N",    "TFTP option 'tsize'", "WRQ without value use calculated"},
  };

}

// -----------------------------------------------------------------------------

/** \brief Class for manipulate TFTP client settings
 *
 *  Parse settings from argc/argv
 */
class ClientSettings: public Options
{
protected:

  ArgParser ap_; ///< Argument parser

  Addr srv_addr_; ///< Server address/port

  int verb_; ///< Verbosity mode

  std::string file_local_; ///< Local filename

  std::string file_remote_; ///< Remote filename

  fLogMsg callback_log_; ///< Logging callback

  /** \brief Default constructor (hide)
   */
  ClientSettings();

  /** \brief Local used logger method
   *
   *  Safe use
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  void log(LogLvl lvl, std::string_view msg) const;

public:

  /** \brief Constructor with logger
   */
  ClientSettings(fLogMsg cb_logger);

  /** \brief Destructor
   */
  virtual ~ClientSettings();

  /** \brief Load settings from CMD arguments
   *
   *  \param [in] argc Count of elements in argv
   *  \param [in] argv Array of arguments
   *  \return True if success, else - false
   */
  bool load_options(int argc, char * argv[]);

  /** \brief Out to cout header output for application
   *
   *  Used for output: Application name, License, etc. See at ArgItems
   *  Get information from ArgItems value
   */
  void out_header() const;

  /** \brief Out to cout help information of arguments options
   *
   *  Used for help output
   *  Get information from ArgItems value
   */
  void out_help() const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPCLIENTSETTINGS_H_ */
