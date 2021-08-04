/*
 * tftpSrvSettings.h
 *
 *  Created on: 4 авг. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPSRVSETTINGS_H_
#define SOURCE_TFTPSRVSETTINGS_H_

#include "tftpCommon.h"
#include "tftpSrvSettingsStor.h"

namespace tftp
{

// -----------------------------------------------------------------------------

class SrvSettings
{
protected:

  pSrvSettingsStor settings_;

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

public:

  SrvSettings();

  SrvSettings(const pSrvSettingsStor & sett);

  SrvSettings(const SrvSettings & src);

  auto operator=(const pSrvSettingsStor & sett) -> SrvSettings &;

  auto operator=(const SrvSettings & src) -> SrvSettings &;

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

  // Getters -------------------------------------------------------------------

  /** \brief Get pointer to SrvSettingsStor instance
   *
   *  \return Shared pointer
   */
  auto get_ptr() const -> const pSrvSettingsStor &;

  auto get_ptr() -> pSrvSettingsStor &;

  /** \brief Get local base (main server listen address)
   *
   *  Safe use
   *  /return Clone of address
   */
  auto server_addr() const -> Addr;

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

  /** \brief Get: Local base address and port as string
   *
   *  Safe use
   *  \return Address and port
   */
  auto get_local_base_str() const -> std::string;

  /** \brief get chmod value
   *
   *  Safe use
   *  \return Value
   */
  int get_file_chmod() const;

  /** \brief get chown user value
   *
   *  Safe use
   *  \return User name
   */
  auto get_file_chown_user() const -> std::string;

  /** \brief get chown group value
   *
   *  Safe use
   *  \return Group name
   */
  auto get_file_chown_grp() const -> std::string;

  // Out stream info -----------------------------------------------------------

  /** \brief Output to stream options help information
   *
   *  Safe use
   *  \param [in] stream Output stream
   *  \param [in] app Application name
   */
  void out_help(std::ostream & stream, std::string_view app) const;

  /** \brief Output to stream base id info
   *
   *  Safe use
   *  \param [out] stream Output stream
   */
  void out_id(std::ostream & stream) const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSRVSETTINGS_H_ */
