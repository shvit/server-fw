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
    Enabler(): SrvSettingsStor() {}
  };

  return std::make_shared<Enabler>();
}

//------------------------------------------------------------------------------

SrvSettingsStor::SrvSettingsStor():
  mutex_{},
  ap_{constants::srv_arg_settings},
  is_daemon{false},
  //local_addr{},
  root_dir{},
  search_dirs{},
  verb{constants::default_tftp_syslog_lvl},
  retransmit_count_{constants::default_retransmit_count},
  file_new_attr{},
  lib_dir{},
  lib_name{constants::default_fb_lib_name},
  db{},
  user{},
  pass{},
  role{},
  dialect{constants::default_fb_dialect}
{
  //local_addr.set_family(AF_INET);
  //local_addr.set_port(constants::default_tftp_port);
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

auto SrvSettingsStor::load_options(
    fLogMsg cb_log,
    ArgParser ap) -> TripleResult
{
  auto log = [&](const LogLvl lvl, std::string_view msg)
    {
      if(cb_log) cb_log(lvl, msg);
    };

  L_DBG("Start server arguments parse");

  TripleResult ret = TripleResult::ok;

  const auto & res = ap.result();

  // 1 Parse options
  for(const auto & item : res.first)
  {
    auto [res_chk, res_str] = ap.chk_parsed_item(item.first);

    switch(res_chk)
    {
      case ResCheck::err_wrong_data:  // wrong do, check code!
      case ResCheck::not_found:       // wrong do, check code!
      case ResCheck::err_no_req_value:
        L_ERR(res_str);
        ret = TripleResult::fail;
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
        //local_addr.set_string(ap.get_parsed_item(item.first));
        break;
      case 2: // help
        ret = TripleResult::nop;
        break;
      case 3: // verb, syslog
        {
          verb = 7;  // default
          auto tmp_val= ap.get_parsed_int(item.first);
          if(tmp_val.has_value()) verb = tmp_val.value();
        }
        break;
      case 4: // lib-dir
        lib_dir = ap.get_parsed_item(item.first);
        break;
      case 5: // lib-name
        lib_name = ap.get_parsed_item(item.first);
        break;
      case 6: // root-dir
        root_dir = ap.get_parsed_item(item.first);
        break;
      case 7: // search
        search_dirs = std::move(ap.get_parsed_items(item.first));
        break;
      case 8: // fb-db
        db = ap.get_parsed_item(item.first);
        break;
      case 9: // fb-user
        user = ap.get_parsed_item(item.first);
        break;
      case 10: // fb-pass
        pass = ap.get_parsed_item(item.first);
        break;
      case 11: // fb-role
        role = ap.get_parsed_item(item.first);
        break;
      case 12: // fb-dialect
        {
          dialect = constants::default_fb_dialect;  // default
          auto tmp_val= ap.get_parsed_int(item.first);
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
          auto tmp_val= ap.get_parsed_int(item.first);
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
        file_new_attr.set_own_user(ap.get_parsed_item(item.first));
        break;
      case 16: // file-chgrp
        file_new_attr.set_own_grp(ap.get_parsed_item(item.first));
        break;
      case 17: // file-chmod
        {
          std::size_t * pos=nullptr;
          file_new_attr.set_mode(std::stoi(ap.get_parsed_item(item.first), pos, 8));
        }
        break;

      default:
        break;
    }
  }

  // 2 Check main arguments

  // ... 2.1 Check listening addresses
  if(auto cnt=res.second.size(); cnt == 0U)
  {
    L_WRN("No any listening server address found");
  }
  else
  {
    //local_addr.set_string(res.second[0U]); // TODO: Remove after fix multi listening

    if(cnt > 1U) L_DBG("Many listening addresses found ("+std::to_string(cnt)+")");
  }

  // ... 2.2 Check root dir
  if((ret == TripleResult::ok) && (root_dir.size() == 0U))
  {
    L_ERR("Not set root server directory");
    ret =  TripleResult::fail;
  }

  L_DBG("Finish server argument parse is "+(ret != TripleResult::fail ? "SUCCESS" : "FAIL"));

  return ret;
}

//------------------------------------------------------------------------------

bool SrvSettingsStor::load_options(fLogMsg cb_log, int argc, char * argv[])
{
  ArgParser ap{constants::srv_arg_settings};

  ap.run(argc, argv);

  return load_options(cb_log, ap) == TripleResult::ok;
}

//------------------------------------------------------------------------------

void SrvSettingsStor::out_help(std::ostream & stream, std::string_view app) const
{
  ap_.out_help(stream, app);
}

// -----------------------------------------------------------------------------

void SrvSettingsStor::out_id(std::ostream & stream) const
{
  ap_.out_header(stream);
}

// -----------------------------------------------------------------------------

} // namespace tftp
