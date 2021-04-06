/*
 * tftpSettings.cpp
 *
 *  Created on: 6 апр. 2021 г.
 *      Author: svv
 */

#include "tftpSettings.h"

namespace tftp
{

//------------------------------------------------------------------------------

settings_val::settings_val():
  is_daemon{false},
  local_base_{},
  lib_dir{},
  lib_name{},
  root_dir{},
  backup_dirs{},
  db{},
  user{},
  pass{},
  role{},
  dialect{},
  use_syslog{},
  log_{nullptr}
{
}

//------------------------------------------------------------------------------

} // namespace tftp
