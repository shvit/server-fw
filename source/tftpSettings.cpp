/**
 * \file tftpSettings.cpp
 * \brief TFTP Settings class module
 *
 *  Storage class for TFTP settings
 *
 *  License GPL-3.0
 *
 *  \date   06-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include "tftpSettings.h"

namespace tftp
{

// -----------------------------------------------------------------------------

auto Settings::create() -> pSettings
{
  return std::make_shared<Settings>(Settings{});
}

//------------------------------------------------------------------------------

Settings::Settings():
  is_daemon{false},
  local_base_{},
  lib_dir{},
  lib_name{default_fb_lib_name},
  root_dir{},
  backup_dirs{},
  db{},
  user{},
  pass{},
  role{},
  dialect{default_fb_dialect},
  use_syslog{default_tftp_syslog_lvl},
  log_{nullptr}
{
}

//------------------------------------------------------------------------------

Settings::~Settings()
{
}

//------------------------------------------------------------------------------

} // namespace tftp
