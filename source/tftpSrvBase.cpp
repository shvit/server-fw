/**
 * \file tftpSrvBase.cpp
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

#include "tftpSrvBase.h"

namespace tftp
{

// -----------------------------------------------------------------------------

SrvBase::SrvBase():
    SrvSettings(SrvSettingsStor::create()),
    Logger(nullptr)
{
}

// -----------------------------------------------------------------------------

SrvBase::SrvBase(const SrvBase & src):
    SrvSettings(src.settings_),
    Logger(src.log_)
{
}

// -----------------------------------------------------------------------------

auto SrvBase::operator=(const SrvBase & src) -> SrvBase &
{
  settings_ = src.settings_;
  log_ = src.log_;

  return *this;
}

// -----------------------------------------------------------------------------

SrvBase::~SrvBase()
{
}

// -----------------------------------------------------------------------------

} // namespace tftp
