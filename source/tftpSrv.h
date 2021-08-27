/**
 * \file tftp_server.h
 * \brief TFTP server class header
 *
 *  TFTP server master class header
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTP_SERVER_H_
#define SOURCE_TFTP_SERVER_H_

#include <list>
#include <thread>

#include "tftpSrvSession.h"
#include "tftpSmBuf.h"
#include "tftpSrvSettings.h"
#include "tftpLogger.h"

namespace tftp
{

// -----------------------------------------------------------------------------

using RuntimeSrvSession = std::pair<pSrvSession, std::thread>;

using RuntimeSrvSessions = std::list<RuntimeSrvSession>;

// -----------------------------------------------------------------------------

/**
 * \brief TFTP main server class 'tftp::Srv'
 *
 *  Class for one listening pair IP:PORT.
 *  Parent class tftp::SrvBase has all server settings storage.
 *
 */

class Srv: public SrvSettings, public Logger
{
protected:

  Addr local_addr_; ///< Local listening address

  RuntimeSrvSessions sessions_; ///< List of running sessions

  int socket_;  ///< Socket for tftp  port listener

  std::atomic_bool stop_; ///< Flag "need stop"

  std::atomic_bool stopped_; ///< Flag "was stopped"

  /** \brief Open socket and listening
   *
   *  \return True if success, false if error occured
   */
  bool socket_open();

  /** \brief Close socket
   */
  void socket_close();

  /** \brief Constructor
   *
   *  \param [in] logger Callback for logging
   *  \param [in] sett Settings for server
   */
  Srv(fLogMsg logger, SrvSettings & sett);

public:

  Srv() = delete; // no default

  /** \brief Main server creator
   *
   *  \param [in] logger Callback for logging
   *  \param [in] sett Settings for server
   *  \return Pointer (uniq) to server instance
   */
  static auto create(fLogMsg logger, SrvSettings & sett) -> pSrv;

  /** \brief Destructor
   */
  virtual ~Srv();

  /** \brief Initalise server method
   *
   *  Can do reinitialize too
   *  Not close running session (to do or not to do)
   */
  bool init(std::string_view list_addr);

  /** \brief Main server loop
   *
   *  At begin run init()
   *  For exit loop outside use Srv::stop()
   */
  void main_loop();

  /** \brief Set exit flag for break main_loop()
   */
  void stop();

  /** \brief Checker for server was stoppped
   *
   *  \return True if stopped, else - false
   */
  bool is_stopped() const;
};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_SERVER_H_ */
