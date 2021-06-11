/*
 * tftpClientSettings.h
 *
 *  Created on: 9 июн. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPCLIENTSETTINGS_H_
#define SOURCE_TFTPCLIENTSETTINGS_H_

#include "tftpCommon.h"
#include "tftpAddr.h"
#include "tftpOptions.h"
#include "tftpArgParser.h"

namespace tftp
{


//----------------------

const ArgItems PPPP
{
  {1, {{"l"}, {"L"}, {"local"}},       ArgExistVaue::required, "Local file name"},
  {2, {{"r"}, {"R"}, {"remote"}},      ArgExistVaue::required, "Remote file name"},
  {3, {{"h"}, {"H"}, {"help"}, {"?"}}, ArgExistVaue::no,       "Show help information"},
  {4, {{"g"}, {"G"}, {"get"}},         ArgExistVaue::no,       "Get file from server"},
  {5, {{"p"}, {"P"}, {"put"}},         ArgExistVaue::no,       "Put file to server"},
};

// -----------------------------------------------------------------------------

class ClientSettings: public Options
{
protected:

  Addr srv_addr_;

  bool verb_;

  std::string file_local_;

  std::string file_remote_;

  fLogMsg callback_log_;

  /** \brief Default constructor (hide)
   */
  ClientSettings();

public:

  /** \brief Public constructing method
   *
   *  \return Unique pointer of ClientSettings
   */
  static auto create() -> pClientSettings;

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

  void out_id(std::ostream & stream) const;

  void out_help(std::ostream & stream, std::string_view app) const;

  /** \brief Local used logger method
   *
   *  Safe use
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  void log(LogLvl lvl, std::string_view msg) const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPCLIENTSETTINGS_H_ */
