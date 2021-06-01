/**
 * \file tftpSettings.cpp
 * \brief TFTP Settings class module
 *
 *  Storage class for TFTP settings
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <getopt.h>
#include <syslog.h>
#include <iostream>
#include <regex>

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
  root_dir{},
  backup_dirs{},
  lib_dir{},
  lib_name{constants::default_fb_lib_name},
  db{},
  user{},
  pass{},
  role{},
  dialect{constants::default_fb_dialect},
  use_syslog{constants::default_tftp_syslog_lvl},
  log_{nullptr},
  retransmit_count_{constants::default_retransmit_count},
  file_chown_user{},
  file_chown_grp{},
  file_chmod{constants::default_file_chmod}
{
  local_base_.set_family(AF_INET);
  local_base_.set_port(constants::default_tftp_port);
}

//------------------------------------------------------------------------------

Settings::~Settings()
{
}

//------------------------------------------------------------------------------

bool Settings::load_options(int argc, char * argv[])
{
  bool ret = true;

  static const char *optString = "l:L:s:S:h?";

  static const struct option longOpts[] =
  {
      { "listen",     required_argument, NULL, 'l' }, // 0
      { "ip",         required_argument, NULL, 'l' }, // 1
      { "syslog",     required_argument, NULL, 's' }, // 2
      { "help",             no_argument, NULL, 'h' }, // 3
      { "lib-dir",    required_argument, NULL,  0  }, // 4
      { "lib-name",   required_argument, NULL,  0  }, // 5
      { "root-dir",   required_argument, NULL,  0  }, // 6
      { "search",     required_argument, NULL,  0  }, // 7
      { "fb-db",      required_argument, NULL,  0  }, // 8
      { "fb-user",    required_argument, NULL,  0  }, // 9
      { "fb-pass",    required_argument, NULL,  0  }, // 10
      { "fb-role",    required_argument, NULL,  0  }, // 11
      { "fb-dialect", required_argument, NULL,  0  }, // 12
      { "daemon",           no_argument, NULL,  0  }, // 13
      { "retransmit", required_argument, NULL,  0  }, // 14
      { "file-chuser",required_argument, NULL,  0  }, // 15
      { "file-chgrp", required_argument, NULL,  0  }, // 16
      { "file-chmod", required_argument, NULL,  0  }, // 17
      { NULL,               no_argument, NULL,  0  }  // always last
  };

  backup_dirs.clear();

  optind=1;
  while(argc > 1)
  {
    int longIndex;
    int opt = getopt_long(argc, argv, optString, longOpts, & longIndex);
    if(opt == -1) break; // end parsing
    switch(opt)
    {
    case 'l':
      if(optarg) local_base_.set_string(std::string{optarg});
      break;
    case 's':
    case 'S':
      {
        if(optarg)
        {
          try
          {
            int lvl = LOG_PRI(std::stoi(optarg));
            use_syslog = lvl;
          } catch (...) { };
        }
      }
      break;
    case 'h':
    case 'H':
    case '?':
      ret = false; // Help message cout
      break;
    case 0:

      switch(longIndex)
      {
      case 4: // --lib-dir
        if(optarg) lib_dir.assign(optarg);
        break;
      case 5: // --lib-name
        if(optarg) lib_name.assign(optarg);
        break;
      case 6: // --root-dir
        if(optarg) root_dir.assign(optarg);
        break;
      case 7: // --search
        if(optarg) backup_dirs.emplace_back(optarg);
        break;
      case 8: // --fb-db
        if(optarg) db.assign(optarg);
        break;
      case 9: // --fb-user
        if(optarg) user.assign(optarg);
        break;
      case 10: // --fb-pass
        if(optarg) pass.assign(optarg);
        break;
      case 11: // --fb-role
        if(optarg) role.assign(optarg);
        break;
      case 12: // --fb-dialect
        if(optarg)
        {
          try
          {
            uint16_t dial = (std::stoul(optarg) & 0xFFFFU);
            dialect = dial;
          } catch (...) { };
        }
        break;
      case 13: // --daemon
        is_daemon = true;
        break;
      case 14: // --retransmit
        if(optarg)
        {
          try
          {
            uint16_t rtr = (std::stoul(optarg) & 0xFFFFU);
            retransmit_count_ = rtr;
          } catch (...) { };
        }
        break;
      case 15: // --file-chuser
        if(optarg) file_chown_user.assign(optarg);
        break;
      case 16: // --file-chgrp
        if(optarg) file_chown_grp.assign(optarg);
        break;
      case 17: // --file-chmod
        if(optarg)
        {
          std::string tmp_str{optarg};
          std::size_t * pos=nullptr;
          file_chmod = std::stoi(tmp_str, pos, 8);
        }
        break;

      } // case (for long option)
      break;
    } // switch
  }

  return ret;
}

//------------------------------------------------------------------------------

void Settings::out_help(std::ostream & stream, std::string_view app) const
{
  out_id(stream);

  stream << "Some features:" << std::endl
  << "  - Recursive search requested files by md5 sum in search directory" << std::endl
  << "  - Use Firebird SQL server as file storage (optional requirement)" << std::endl
  << "Usage: " << app << " [<option1> [<option1 argument>]] [<option2> [<option2 argument>]] ... " << std::endl
  << "Possible options:" << std::endl
  << "  {-h|-H|-?|--help} Show help message" << std::endl
  << "  {-l|-L|--ip|--listen} {<IPv4>|[<IPv6>]}[:port] Listening address and port" << std::endl
  << "    (default 0.0.0.0:" << constants::default_tftp_port << ")" << std::endl
  << "    Sample IPv4: 192.168.0.1:69" << std::endl
  << "    Sample IPv6: [::1]:69" << std::endl
  << "  {-s|-S|--syslog} <0...7> SYSLOG level flooding (default " << constants::default_tftp_syslog_lvl << ")" << std::endl
  << "  --lib-dir <directory> System library directory" << std::endl
  << "  --lib-name <name> Firebird library filename (default " << constants::default_fb_lib_name << ")" << std::endl
  << "  --root-dir <directory> Server root directory" << std::endl
  << "  --search <directory> Directory for recursive search by md5 sum (may be much)" << std::endl
  << "  --fb-db <database> Firebird access database name" << std::endl
  << "  --fb-user <username> Firebird access user name" << std::endl
  << "  --fb-pass <password> Firebird access password" << std::endl
  << "  --fb-role <role> Firebird access role" << std::endl
  << "  --fb-dialect <1...3> Firebird server dialect (default " << constants::default_fb_dialect << ")" << std::endl
  << "  --daemon Run as daemon" << std::endl
  << "  --retransmit <N> Maximum retransmit count if fail" << std::endl
  << "  --file-chuser <user name> Set user owner for created files (default root)" << std::endl
  << "  --file-chgrp <group name> Set group owner for created files (default root)" << std::endl
  << "    Warning: if user/group not exist then use root" << std::endl
  << "  --file-chmod <permissions> Set permissions for created files (default 0664)" << std::endl
  << "    Warning: can set only r/w bits - maximum 0666; can't set x-bits and superbits" << std::endl;
}

// -----------------------------------------------------------------------------

void Settings::out_id(std::ostream & stream) const
{
  stream << "Simple tftp firmware server 'server-fw' v0.2 licensed GPL-3.0" << std::endl
  << "(c) 2019-2021 Vitaliy.V.Shirinkin, e-mail: vitaliy.shirinkin@gmail.com" << std::endl;
}

// -----------------------------------------------------------------------------

} // namespace tftp
