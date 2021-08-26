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

public:

  /** \brief Default constructor
   */
  SrvSettings();

  /** \brief Constructor from storage
   *
   *  \param [in] sett Pointer to settings
   */
  SrvSettings(const pSrvSettingsStor & sett);

  /** \brief Copy constructor
   *
   *  \param [in] src Self type source
   */
  SrvSettings(const SrvSettings & src);

  /** \brief Setter for storage
   *
   *  \param [in] sett Pointer to settings
   *  \return Self reference
   */
  auto operator=(const pSrvSettingsStor & sett) -> SrvSettings &;

  /** \brief Copy operator
   *
   *  \param [in] src Self type source
   *  \return Self reference
   */
  auto operator=(const SrvSettings & src) -> SrvSettings &;

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
  //auto server_addr() const -> Addr;

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
  //auto get_local_addr_str() const -> std::string;

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
  //void out_help(std::ostream & stream, std::string_view app) const;

  /** \brief Output to stream base id info
   *
   *  Safe use
   *  \param [out] stream Output stream
   */
  //void out_id(std::ostream & stream) const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPSRVSETTINGS_H_ */
