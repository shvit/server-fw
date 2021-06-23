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
  ap_{constants::arg_option_settings},
  srv_addr{},
  verb{4},
  file_local{},
  //file_remote{},
  opt{}
{
  srv_addr.set_string("127.0.0.1:"+std::to_string(constants::default_tftp_port));
}

// -----------------------------------------------------------------------------

ClientSettings::~ClientSettings()
{
}

//------------------------------------------------------------------------------

bool ClientSettings::load_options(fLogMsg log, int argc, char * argv[])
{
  L_DBG("Start argument parse (argc is "+std::to_string(argc)+")");

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
      case 1: // local file
        file_local = ap_.get_parsed_item(item.first);
        break;
      case 2: // remote file
        opt.set_filename(ap_.get_parsed_item(item.first), log);
        break;
      case 3: // Request GET
        opt.set_request_type(SrvReq::read);
        break;
      case 4: // Request PUT
        opt.set_request_type(SrvReq::write);
        break;
      case 5: // Help
        ret = false;
        //out_help();
        break;
      case 6: // Verbosity
        {
          verb = 7;  // default
          auto tmp_val= ap_.get_parsed_int(item.first);
          if(tmp_val.has_value()) verb = tmp_val.value();
        }
        break;
      case 7: // Mode transfer
        opt.set_transfer_mode(ap_.get_parsed_item(item.first), log);
        break;
      case 8: // Block size
        opt.set_blksize(ap_.get_parsed_item(item.first), log);
        break;
      case 9: // Timeout
        opt.set_timeout(ap_.get_parsed_item(item.first), log);
        break;
      case 10: // Windowsize
        opt.set_windowsize(ap_.get_parsed_item(item.first), log);
        break;
      case 11: // Tsize
        opt.set_tsize(ap_.get_parsed_item(item.first), log);
        break;
      default:
        break;
    }
  }

  // 2 Parse main arguments
  if(auto cnt=res.second.size(); cnt == 0U)
  {
    L_WRN("No server address found; used "+srv_addr.str());
  }
  else
  if(cnt > 1U)
  {
    L_ERR("Too many address found ("+std::to_string(cnt)+")");
  }
  else
  {
    srv_addr.set_string(res.second[0U]);
  }

  L_DBG("Finish argument parse is "+(ret ? "SUCCESS" : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

void ClientSettings::out_header(std::ostream & stream) const
{
  ap_.out_header(stream);
}

// -----------------------------------------------------------------------------

void ClientSettings::out_help(std::ostream & stream) const
{
  ap_.out_help(stream);
}

// -----------------------------------------------------------------------------

} // namespace tftp


