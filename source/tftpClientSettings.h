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
#include "tftpLogger.h"

namespace tftp
{

//------------------------------------------------------------------------------

namespace constants
{

  /** \brief Settings for parsing CMD argument options
   *
   *  Used for parse with class ArgParser
   */
  const ArgItems client_arg_settings =
  {
    {0, {}, ArgExistVaue::no, "", "Simple TFTP client from 'server-fw' project licensed GPL-3.0", ""},
    {0, {}, ArgExistVaue::no, "", "Github project page https://github.com/shvit/server-fw", ""},
    {0, {}, ArgExistVaue::no, "", "--", ""},
    {0, {}, ArgExistVaue::no, "", "Usage:", ""},
    {0, {}, ArgExistVaue::no, "", "./tftp-cl [<options ...>]", "<IP addr>[:<UDP Port>]"},
    {0, {}, ArgExistVaue::no, "", "Possible options:", ""},
    { 1, {"l", "L", "local"},      ArgExistVaue::required, "file", "Local file path and name", ""},
    { 2, {"r", "R", "remote"},     ArgExistVaue::required, "file", "Remote file name", ""},
    { 3, {"g", "G", "get"},        ArgExistVaue::no,       "",     "Get file from server", ""},
    { 4, {"p", "P", "put"},        ArgExistVaue::no,       "",     "Put file to server", ""},
    { 5, {"h", "H", "help", "?"},  ArgExistVaue::no,       "",     "Show help information", ""},
    { 6, {"v", "V", "verb"},       ArgExistVaue::optional, "level", "Set verbosity mode with logging level", "default 7 debug"},
    { 7, {"m", "M", "mode"},       ArgExistVaue::required, "mode", "TFTP transfer mode", ""},
    { 8, {"b", "B", "blksize"},    ArgExistVaue::required, "N",    "TFTP option 'block size'", "default 512"},
    { 9, {"t", "T", "timeout"},    ArgExistVaue::required, "N",    "TFTP option 'timeout'", "default 10"},
    {10, {"w", "W", "windowsize"}, ArgExistVaue::required, "N",    "TFTP option 'windowsize'", "default 1"},
    {11, {"Z", "z", "tsize"},      ArgExistVaue::optional, "N",    "TFTP option 'tsize'", "WRQ without value use calculated"},
  };

}

// -----------------------------------------------------------------------------

/** \brief Class for manipulate TFTP client settings
 *
 *  Parse settings from argc/argv
 */
class ClientSettings
{
protected:

  /** \brief Default constructor
   */
  ClientSettings();

public:

  Addr srv_addr; ///< Server address/port

  int verb; ///< Verbosity mode

  std::string file_local; ///< Local filename

  Options opt; ///< Options with remote filename

  /** \brief Local used logger method
   *
   *  Safe use
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  //void log(LogLvl lvl, std::string_view msg) const;

  static auto create() -> pClientSettings;

  /** \brief Destructor
   */
  virtual ~ClientSettings();

  /** \brief Load settings from CMD arguments
   *
   *  \param [in] cb_logger Callback logger
   *  \param [in] ap Instance of parser
   *  \return 'nop' - normal exit,
   *          'ok' - normal contimue,
   *          'fail' - fail exit
   */
  auto load_options(
      fLogMsg cb_logger,
      ArgParser & ap) -> TripleResult;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPCLIENTSETTINGS_H_ */
