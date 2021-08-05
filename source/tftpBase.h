/**
 * \file tftpBase.h
 * \brief Base TFTP server class header
 *
 *  Base class for TFTP server
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPBASE_H_
#define SOURCE_TFTPBASE_H_


#include "tftpCommon.h"
#include "tftpSrvSettings.h"
#include "tftpLogger.h"

namespace tftp
{

// -----------------------------------------------------------------------------

/**
 * \brief Base class for child TFTP server classes
 *
 *  Main functionality:
 *  - Access to server settings
 *  - Logging messages
 */
class Base: public SrvSettings, public Logger
{
protected:

  /** \brief Any allowed type setter
   *
   *  Now allow only:
   *  - SrvSettings (inherited from SrvSettings)
   *  - pSrvSettingsStor (inherited from SrvSettings)
   *  - fLogMsg (inherited from Logger)
   *  - any child classes from Base
   *  For unknown type throw exception std::runtime_error
   *  \param [in] arg Source value
   */
  template<typename T>
  void set_value(T && arg);

public:

  /** \brief Default constructor
   */
  Base();

  /** \brief Copy constructor
   */
  Base(const Base &);

  /** \brief Any allowed type constructor
   *
   *  Allowed type see at function set_value()
   *  All arguments pass to set_value()
   *  \param [in] args Variadic arguments
   */
  template<typename ... Ts>
  Base(Ts && ... args);

  /** Destructor
   */
  virtual ~Base();

  /** \brief Copy operator self type
   *
   *  \param [in] src Other Base value
   *  \return Self reference
   */
  auto operator=(const Base & src) -> Base &;

};

// -----------------------------------------------------------------------------

template<typename T>
void Base::set_value(T && arg)
{
  using TT = std::decay_t<T>;
  if constexpr (std::is_same_v<TT, SrvSettings> ||
                std::is_same_v<TT, pSrvSettingsStor> ||
                std::is_same_v<TT, fLogMsg>)
  {
    operator=(std::forward<T>(arg));
  }
  else
  if constexpr (std::is_convertible_v<T, Base>)
  {
    operator=(std::forward<T>(arg));
  }
  else // debug section
  {
    throw std::runtime_error("Unknown initialize type");
  }
}

// -----------------------------------------------------------------------------

template<typename ... Ts>
Base::Base(Ts && ... args):
    Base()
{
  (set_value(std::forward<Ts>(args)), ...);
}

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPBASE_H_ */
