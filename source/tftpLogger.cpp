/**
 * \file tftpLogger.cpp
 * \brief Logger base module
 *
 *  License GPL-3.0
 *
 *  \date 03-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
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

Logger::Logger(const Logger & src)
{
  log_ = src.log_;
}

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

auto Logger::operator=(const Logger & src) -> Logger &
{
  log_ = src.log_;
  return *this;
}

//------------------------------------------------------------------------------

void Logger::log(LogLvl lvl, std::string_view msg) const
{
  if(log_) log_(lvl, msg);
}

//------------------------------------------------------------------------------

} // namespace tftp
