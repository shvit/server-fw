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

#include "tftpSession.h"
#include "tftpSmBuf.h"
#include "tftpSrvSettings.h"
#include "tftpLogger.h"

namespace tftp
{

using RuntimeSession = std::pair<pSession, std::thread>;

using RuntimeSessions = std::list<RuntimeSession>;

// -----------------------------------------------------------------------------

/**
 * \brief TFTP main server class 'tftp::Srv'
 *
 *  Class for one listening pair IP:PORT.
 *  Parent class tftp::Base has all server settings storage.
 *
 */

class Srv: public Base
{
protected:

  RuntimeSessions sessions_; ///< List of running sessions

  int socket_;  ///< Socket for tftp  port listener

  bool stop_; ///< Flag "need stop"

  /** \brief Open socket and listening
   *
   *  \return True if success, false if error occured
   */
  bool socket_open();

  /** \brief Close socket
   */
  void socket_close();

public:

  /** \brief Default constructor
   */
  Srv();

  //Srv(fLogMsg cb_log);

  /** \brief Destructor
   */
  virtual ~Srv();

  /** \brief Initalise server method
   *
   *  Can do reinitialize too
   *  Not close running session (to do or not to do)
   */
  bool init();

  /** \brief Main server loop
   *
   *  At begin run init()
   *  For exit loop outside use Srv::stop()
   */
  void main_loop();

  /** \brief Set exit flag for break inside main_loop()
   */
  void stop();

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_SERVER_H_ */
