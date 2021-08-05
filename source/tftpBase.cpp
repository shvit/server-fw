/**
 * \file tftpBase.h
 * \brief Base TFTP server class module
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

#include <unistd.h>
#include <sys/syscall.h>
#include <syslog.h>

#include "tftpBase.h"

namespace tftp
{

// -----------------------------------------------------------------------------

Base::Base():
    SrvSettings(SrvSettingsStor::create()),
    Logger(nullptr)
{
}

// -----------------------------------------------------------------------------

Base::Base(const Base & src):
    SrvSettings(src.settings_),
    Logger(src.log_)
{
}

// -----------------------------------------------------------------------------

auto Base::operator=(const Base & src) -> Base &
{
  settings_ = src.settings_;
  log_ = src.log_;

  return *this;
}

// -----------------------------------------------------------------------------

Base::~Base()
{
}

// -----------------------------------------------------------------------------

} // namespace tftp
