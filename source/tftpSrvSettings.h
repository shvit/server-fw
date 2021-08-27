/**
 * \file tftpSrvSettings.h
 * \brief SrvSettings class header
 *
 *  License GPL-3.0
 *
 *  \date 04-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPSRVSETTINGS_H_
#define SOURCE_TFTPSRVSETTINGS_H_

#include "tftpCommon.h"
#include "tftpSrvSettingsStor.h"

namespace tftp
{

// -----------------------------------------------------------------------------

/** \brief Server settings helper
 *
 *  Use for easy access to server storage class
 */
class SrvSettings
{
protected:

  pSrvSettingsStor settings_; ///< Server settings storage

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

  /** \brief Constructor from storage
   *
   *  \param [in] sett Pointer to settings
   */
  SrvSettings(const pSrvSettingsStor & sett);

public:

  /** \brief Default constructor
   */
  SrvSettings();

  /** \brief Copy constructor
   *
   *  \param [in] src Self type source
   */
  SrvSettings(const SrvSettings & src);

  /** \brief Copy operator
   *
   *  \param [in] src Self type source
   *  \return Self reference
   */
  auto operator=(const SrvSettings & src) -> SrvSettings &;

  /** \brief Load options from parseg arguments
   *
   *  Safe use - unique lock
   *  \param [in] cb_logger Callback logger
   *  \param [in] ap Instance of parser
   *  \return 'nop' - normal exit,
   *          'ok' - normal contimue,
   *          'fail' - fail exit
   */
  auto load_options(
      fLogMsg cb_logger,
      ArgParser ap) -> TripleResult;

  // Getters -------------------------------------------------------------------

  /** \brief Get: root server directory
   *
   *  Safe use
   *  \return Path to root server directory
   */
  auto get_root_dir() const -> std::string;

  /** \brief Get: system library directory
   *
   *  Safe use
   *  \return Path to system library directory
   */
  auto get_lib_dir() const -> std::string;

  /** \brief Get: Firebird library name
   *
   *  Safe use
   *  \return Name Firebird library name
   */
  auto get_lib_name_fb() const -> std::string;

  /** \brief Get retransmit count
   *
   *  Safe use
   *  \return Vlaue
   */
  auto get_retransmit_count() const -> uint16_t;

  /** \brief Get: Flag is daemon
   *
   *  Safe use
   *  \return Flag is daemon
   */
  bool get_is_daemon() const;

  /** \brief Get searched directory
   *
   *  Safe use
   *  \return Vector with strings
   */
  auto get_serach_dir() const -> VecStr;

  /** \brief Get chmod value
   *
   *  Safe use
   *  \return Value
   */
  int get_file_chmod() const;

  /** \brief Get chown user value
   *
   *  Safe use
   *  \return User name
   */
  auto get_file_chown_user() const -> std::string;

  /** \brief Get chown group value
   *
   *  Safe use
   *  \return Group name
   */
  auto get_file_chown_grp() const -> std::string;

  /** \brief Get verbosity mode
   *
   *  Safe use
   *  \return Verbosity level
   */
  auto get_verb() const -> LogLvl;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSRVSETTINGS_H_ */
