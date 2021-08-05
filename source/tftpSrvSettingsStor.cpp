/**
 * \file tftpSrvSettingsStor.cpp
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

#include "tftpSrvSettingsStor.h"

namespace tftp
{

// -----------------------------------------------------------------------------

auto SrvSettingsStor::create() -> pSrvSettingsStor
{
  class Enabler : public SrvSettingsStor
  {
  public:
    Enabler() : SrvSettingsStor{} {}
  };

  return std::make_shared<Enabler>();
}

//------------------------------------------------------------------------------

SrvSettingsStor::SrvSettingsStor():
  mutex_{},
  ap_{constants::srv_arg_settings},
  is_daemon{false},
  local_addr{},
  root_dir{},
  search_dirs{},
  verb{constants::default_tftp_syslog_lvl},
  retransmit_count_{constants::default_retransmit_count},
  file_new_attr{},
  //file_chown_user{},
  //file_chown_grp{},
  //file_chmod{constants::default_file_chmod},
  lib_dir{},
  lib_name{constants::default_fb_lib_name},
  db{},
  user{},
  pass{},
  role{},
  dialect{constants::default_fb_dialect}
{
  local_addr.set_family(AF_INET);
  local_addr.set_port(constants::default_tftp_port);
}

//------------------------------------------------------------------------------

SrvSettingsStor::~SrvSettingsStor()
{
}

// -----------------------------------------------------------------------------

auto SrvSettingsStor::begin_shared() const
    -> std::shared_lock<std::shared_mutex>
{
  return std::shared_lock{mutex_};
}

// -----------------------------------------------------------------------------

auto SrvSettingsStor::begin_unique() const
    -> std::unique_lock<std::shared_mutex>
{
  return std::unique_lock{mutex_};
}

//------------------------------------------------------------------------------

bool SrvSettingsStor::load_options(fLogMsg cb_log, int argc, char * argv[])
{
  auto log = [&](const LogLvl lvl, std::string_view msg)
    {
      if(cb_log) cb_log(lvl, msg);
    };

  L_DBG("Start argument parse (arg count "+std::to_string(argc)+")");

  bool ret = true;

  const auto & res = ap_.run(
      argc,
      argv);

  // 1 Parse options
  for(const auto & item : res.first)
  {
    auto [res_chk, res_str] = ap_.chk_parsed_item(item.first);

    switch(res_chk)
    {
      case ResCheck::err_wrong_data:  // wrong do, check code!
      case ResCheck::not_found:       // wrong do, check code!
      case ResCheck::err_no_req_value:
        L_ERR(res_str);
        ret = false;
        continue;
      case ResCheck::wrn_many_arg:
        L_WRN(res_str);
        break;
      case ResCheck::normal:
        break;
    }

    switch(item.first)
    {
      case 1: // listen
        local_addr.set_string(ap_.get_parsed_item(item.first));
        break;
      case 2: // help
        ret = false;
        //out_help();
        break;
      case 3: // verb, syslog
        {
          verb = 7;  // default
          auto tmp_val= ap_.get_parsed_int(item.first);
          if(tmp_val.has_value()) verb = tmp_val.value();
        }
        break;
      case 4: // lib-dir
        lib_dir = ap_.get_parsed_item(item.first);
        break;
      case 5: // lib-name
        lib_name = ap_.get_parsed_item(item.first);
        break;
      case 6: // root-dir
        root_dir = ap_.get_parsed_item(item.first);
        break;
      case 7: // search
        search_dirs = std::move(ap_.get_parsed_items(item.first));
        break;
      case 8: // fb-db
        db = ap_.get_parsed_item(item.first);
        break;
      case 9: // fb-user
        user = ap_.get_parsed_item(item.first);
        break;
      case 10: // fb-pass
        pass = ap_.get_parsed_item(item.first);
        break;
      case 11: // fb-role
        role = ap_.get_parsed_item(item.first);
        break;
      case 12: // fb-dialect
        {
          dialect = constants::default_fb_dialect;  // default
          auto tmp_val= ap_.get_parsed_int(item.first);
          if(tmp_val.has_value())
          {
            if((tmp_val.value() >= 1) && (tmp_val.value() <= 3))
            {
              dialect = (uint16_t)tmp_val.value();
            }
          }
        }
        break;
      case 13: // daemon
        is_daemon = true;
        break;
      case 14: // retransmit
        {
          retransmit_count_ = constants::default_retransmit_count;  // default
          auto tmp_val= ap_.get_parsed_int(item.first);
          if(tmp_val.has_value())
          {
            if((tmp_val.value() > 0) && (tmp_val.value() < 65535))
            {
              retransmit_count_ = (uint16_t)tmp_val.value();
            }
          }
        }
        break;
      case 15: // file-chuser
        //file_chown_user = ap_.get_parsed_item(item.first);
        file_new_attr.set_own_user(ap_.get_parsed_item(item.first));
        break;
      case 16: // file-chgrp
        //file_chown_grp = ap_.get_parsed_item(item.first);
        file_new_attr.set_own_grp(ap_.get_parsed_item(item.first));
        break;
      case 17: // file-chmod
        {
          std::size_t * pos=nullptr;
          //file_chmod = std::stoi(ap_.get_parsed_item(item.first), pos, 8) & 0777;
          file_new_attr.set_mode(std::stoi(ap_.get_parsed_item(item.first), pos, 8));
        }
        break;

      default:
        break;
    }
  }

  // 2 Parse main arguments
  if(auto cnt=res.second.size(); cnt == 0U)
  {
    L_WRN("No server address found; used "+local_addr.str());
  }
  else
  if(cnt > 1U)
  {
    L_ERR("Too many address found ("+std::to_string(cnt)+")");
    // TODO: parse for multi listening
  }
  else
  {
    local_addr.set_string(res.second[0U]);
  }

  L_DBG("Finish argument parse is "+(ret ? "SUCCESS" : "FAIL"));

  return ret;
}

//------------------------------------------------------------------------------

void SrvSettingsStor::out_help(std::ostream & stream, std::string_view app) const
{
  ap_.out_help(stream, app);

  /*
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
  */
}

// -----------------------------------------------------------------------------

void SrvSettingsStor::out_id(std::ostream & stream) const
{
  ap_.out_header(stream);
  //stream << "Simple tftp firmware server 'server-fw' v" << constants::app_version << " licensed GPL-3.0" << std::endl
  //<< "(c) 2019-2021 Vitaliy.V.Shirinkin, e-mail: vitaliy.shirinkin@gmail.com" << std::endl;
}

// -----------------------------------------------------------------------------

} // namespace tftp
