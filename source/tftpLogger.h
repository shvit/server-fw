/*
 * tftpLogger.h
 *
 *  Created on: 3 авг. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPLOGGER_H_
#define SOURCE_TFTPLOGGER_H_

#include "tftpCommon.h"

namespace tftp
{

//------------------------------------------------------------------------------

class Logger
{
protected:

  fLogMsg log_; ///< Callback logging to top level

public:

  Logger();

  Logger(fLogMsg new_cb);

  Logger(const Logger & new_cb);

  Logger(Logger && new_cb);

  virtual ~Logger();

  auto operator=(const Logger & new_cb) -> Logger &;

  auto operator=(Logger && new_cb) -> Logger &;

  void log(LogLvl lvl, std::string_view msg) const;

};

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPLOGGER_H_ */
