/*
 * tftpLogger.cpp
 *
 *  Created on: 3 авг. 2021 г.
 *      Author: svv
 */


#include "tftpLogger.h"

namespace tftp
{

//------------------------------------------------------------------------------

Logger::Logger():
    Logger(nullptr)
{
}

//------------------------------------------------------------------------------

Logger::Logger(fLogMsg new_cb):
    log_{new_cb}
{
}

//------------------------------------------------------------------------------

Logger::Logger(const Logger & new_cb)
{
  log_ = new_cb.log_;
}

//------------------------------------------------------------------------------
/*
Logger::Logger(Logger && new_cb)
{
  log_ = new_cb.log_;
  new_cb.log_ = nullptr;
}
*/

//------------------------------------------------------------------------------

auto Logger::operator=(fLogMsg new_cb) -> Logger &
{
  log_ = new_cb;
  return *this;
}

//------------------------------------------------------------------------------

Logger::~Logger()
{
}

//------------------------------------------------------------------------------

auto Logger::operator=(const Logger & new_cb) -> Logger &
{
  log_ = new_cb.log_;
  return *this;
}

//------------------------------------------------------------------------------
/*
auto Logger::operator=(Logger && new_cb) -> Logger &
{
  log_ = new_cb.log_;
  new_cb.log_ = nullptr;
  return *this;
}
*/
//------------------------------------------------------------------------------

void Logger::log(LogLvl lvl, std::string_view msg) const
{
  if(log_) log_(lvl, msg);
}

//------------------------------------------------------------------------------

} // namespace tftp
