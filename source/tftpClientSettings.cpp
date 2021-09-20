/**
 * \file  tftpClientSettingss.cpp
 * \brief Class ClientSettings module
 *
 *  Class for arguments parsing for TFTP client
 *
 *  License GPL-3.0
 *
 *  \date 09-jun-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <iostream>

#include "tftpClientSettings.h"

namespace tftp
{

// -----------------------------------------------------------------------------

ClientSettings::ClientSettings():
  srv_addr{},
  verb{4},
  file_local{},
  opt{}
{
  srv_addr.set_string("127.0.0.1:"+std::to_string(constants::default_tftp_port));
  opt.set_transfer_mode(std::string{to_string(TransfMode::octet)});
}

// -----------------------------------------------------------------------------

auto ClientSettings::create() -> pClientSettings
{
  class Enabler: public ClientSettings {};

  return std::make_unique<Enabler>();
}

// -----------------------------------------------------------------------------

ClientSettings::~ClientSettings()
{
}

//------------------------------------------------------------------------------

auto ClientSettings::load_options(
    fLogMsg cb_log,
    ArgParser & ap) -> TripleResult
{
  auto log = [&](const LogLvl lvl, std::string_view msg)
    {
      if(cb_log) cb_log(lvl, msg);
    };

  L_DBG("Load client arguments started");

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
      case 1: // local file
        file_local = ap.get_parsed_item(item.first);
        L_DBG("Set local filename '"+file_local+"'");
        break;
      case 2: // remote file
        if(opt.set_filename(ap.get_parsed_item(item.first), log))
        {
          L_DBG("Set remote filename '"+opt.filename()+"'");
        }
        break;
      case 3: // Request GET
        opt.set_request_type(SrvReq::read);
        L_DBG("Set request RRQ");
        break;
      case 4: // Request PUT
        opt.set_request_type(SrvReq::write);
        L_DBG("Set request WRQ");
        break;
      case 5: // Help
        ret = TripleResult::nop;
        break;
      case 6: // Verbosity
        {
          verb = 7;  // default
          auto tmp_val= ap.get_parsed_int(item.first);
          if(tmp_val.has_value()) verb = tmp_val.value();
        }
        break;
      case 7: // Mode transfer
        if(opt.set_transfer_mode(ap.get_parsed_item(item.first), log))
        {
          L_DBG("Set mode '"+opt.transfer_mode()+"'");
        }
        break;
      case 8: // Block size
        if(opt.set_blksize(ap.get_parsed_item(item.first), log))
        {
          L_DBG("Set option blksize="+std::to_string(opt.blksize()));
        }
        break;
      case 9: // Timeout
        if(opt.set_timeout(ap.get_parsed_item(item.first), log))
        {
          L_DBG("Set option timeout="+std::to_string(opt.timeout()));
        }
        break;
      case 10: // Windowsize
        if(opt.set_windowsize(ap.get_parsed_item(item.first), log))
        {
          L_DBG("Set option windowsize="+std::to_string(opt.windowsize()));
        }
        break;
      case 11: // Tsize
        if(opt.set_tsize(ap.get_parsed_item(item.first), log))
        {
          L_DBG("Set option tsize="+std::to_string(opt.tsize()));
        }
        break;
      default:
        break;
    }
  }

  // 2 Parse main arguments
  if(auto cnt=res.second.size(); cnt == 0U)
  {
    L_INF("No server address found; used "+srv_addr.str());
  }
  else
  if(cnt > 1U)
  {
    L_ERR("Too many address found ("+std::to_string(cnt)+")");
  }
  else
  {
    srv_addr.set_string(res.second[0U]);
    L_DBG("Set server as '"+srv_addr.str()+"'");
  }

  L_DBG("Load client arguments finished is "+(ret != TripleResult::fail ? "SUCCESS" : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

} // namespace tftp


