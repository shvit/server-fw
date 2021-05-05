/**
 * \file tftpBase.h
 * \brief Base TFTP server class header
 *
 *  Base class for TFTP server
 *
 *  License GPL-3.0
 *
 *  \date   05-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTPBASE_H_
#define SOURCE_TFTPBASE_H_

#include <shared_mutex>

#include "tftpCommon.h"
#include "tftpSettings.h"

namespace tftp
{

// -----------------------------------------------------------------------------

/**
 * \brief Base class for child TFTP server classes
 *
 * Base class with server settings storage.
 * Thread safe access to storage by read/write mutex.
 * Base logging infrastructure.
 * Base settings getters.
 * Method for parsing running arguments (options).
 *
 * External direct use "settings_" can only with starting:
 * - begin_shared() for settings read operations
 * - begin_unique() for settings write operations
 */
class Base
{
protected:

  /** \brief Shared pointer for settings storage
   *
   *  Danger direct use! Before use do it:
   *  - begin_shared()
   *  - begin_unique()
   */
  pSettings settings_;

  mutable std::shared_mutex mutex_; ///< RW mutex for threading access

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

  /** \brief Constructor from Settings pointer
   *
   *  \param [in] src Settings instance
   */
  Base(const pSettings & sett);

public:

  /** \brief Default constructor
   */
  Base();

  /** \brief Get pointer to Settings instance
   *
   *  \return Shared pointer
   */
  auto get_ptr() const -> const pSettings &;

  /** Destructor
   */
  virtual ~Base();

  /** \brief Copy/move operator
   *
   *  \param [in] src Other Base value
   *  \return Self reference
   */
  auto operator=(const Base & src) -> Base &;

  /** \brief Get local base (main server listen address)
   *
   *  Safe use
   *  /return Clone of address
   */
  auto server_addr() const -> Addr;

  /** \brief Local used logger method
   *
   *  Safe use
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  void log(LogLvl lvl, std::string_view msg) const;

  /** \brief Set: second custom logger
   *
   *  Use mutex unique mode
   *  \param [in] new_logger Custom function of logger
   */
  void set_logger(fLogMsg new_logger);

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

  /** \brief Get: Firebird access information
   *
   *  Safe use
   *  \return std::tuple {database, username, password, role, dialect}
   */
  auto get_connection() const
    -> std::tuple<std::string,
                  std::string,
                  std::string,
                  std::string,
                  uint16_t>;

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

  /** \brief Fill all options from string set
   *
   *  Safe use
   *  \param [in] argc Count of strings
   *  \param [in] argv Strings (char) array
   */
  bool load_options(int argc, char* argv[]);

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

#endif /* SOURCE_TFTPBASE_H_ */
