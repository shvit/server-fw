/**
 * \file tftp_server.h
 * \brief TFTP server class header
 *
 *  TFTP server master class header
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTP_SERVER_H_
#define SOURCE_TFTP_SERVER_H_

#include <list>
#include <string>

#include "tftp_session.h"

namespace tftp
{

/**
 * \brief TFTP main server class 'tftp::srv'
 *
 *  Class for one listening pair IP:PORT.
 *  Parent class tftp::Base has all server settings storage.
 *
 */

class srv: public Base
{
protected:

  /// List of running sessions
  std::list<std::tuple<session, std::thread>> sessions_;

  int      socket_; ///< Socket of listener
  Buf buffer_; ///< Income buffer for tftp request

  bool stop_; ///< Flag "need stop"

  /// Listening socket open and tune
  bool socket_open();

  /// Listening socket close
  void socket_close();

public:
  /// Constructor
  srv();

  /// Destructor
  virtual ~srv();

  /// Initalise server method
  bool init();

  /// Server loop
  void main_loop();

  void stop();

};

// ----------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_SERVER_H_ */
