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

#include <getopt.h>
#include <syslog.h>

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
  retransmit_count_{constants::default_retransmit_count}
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

      } // case (for long option)
      break;
    } // switch
  }

  return ret;
}

//------------------------------------------------------------------------------


} // namespace tftp
