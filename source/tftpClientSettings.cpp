/*
 * tftpClientSettings.cpp
 *
 *  Created on: 9 июн. 2021 г.
 *      Author: svv
 */

//#include <getopt.h>
#include <iostream>

#include "tftpClientSettings.h"

namespace tftp
{

// -----------------------------------------------------------------------------

ClientSettings::ClientSettings():
    Options(),
    ap_{constants::arg_option_settings},
    srv_addr_{},
    verb_{false},
    file_local_{},
    file_remote_{},
    callback_log_{nullptr}
{
  srv_addr_.set_string("127.0.0.1:"+std::to_string(constants::default_tftp_port));
}

// -----------------------------------------------------------------------------

ClientSettings::~ClientSettings()
{
}

//------------------------------------------------------------------------------

bool ClientSettings::load_options(int argc, char * argv[])
{
  L_DBG("Start argument parse (argc is "+std::to_string(argc)+")");

  bool ret = true;

  fLogMsg do_logging = std::bind(
      & ClientSettings::log,
      this,
      std::placeholders::_1,
      std::placeholders::_2);

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
        file_local_ = ap_.get_parsed_item(item.first);
        break;
      case 2: // remote file
        file_remote_ = ap_.get_parsed_item(item.first);
        break;
      case 3: // Request GET
        request_type_ = SrvReq::read;
        break;
      case 4: // Request PUT
        request_type_ = SrvReq::write;
        break;
      case 5: // Help
        ret = false;
        //ArgParser::out_help_data(arg_option_settings, std::cout, argv[0]);
        out_help(std::cout, argv[0]);
        break;
      case 6: // Verbosity
        verb_ = true;
        break;
      case 7: // Mode transfer
        set_transfer_mode(ap_.get_parsed_item(item.first), do_logging);
        break;
      case 8: // Block size
        set_blksize(ap_.get_parsed_item(item.first), do_logging);
        break;
      case 9: // Timeout
        set_timeout(ap_.get_parsed_item(item.first), do_logging);
        break;
      case 10: // Windowsize
        set_windowsize(ap_.get_parsed_item(item.first), do_logging);
        break;
      case 11: // Tsize
        set_tsize(ap_.get_parsed_item(item.first), do_logging);
        break;
      default:
        break;
    }
  }

  // 2 Parse main arguments
  if(auto cnt=res.second.size(); cnt == 0U)
  {
    L_WRN("No server address found; used "+srv_addr_.str());
  }
  else
  if(cnt > 1U)
  {
    L_ERR("Too many address found ("+std::to_string(cnt)+")");
  }
  else
  {
    srv_addr_.set_string(res.second[0U]);
  }

  L_DBG("Finish argument parse is "+(ret ? "SUCCESS" : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

void ClientSettings::log(LogLvl lvl, std::string_view msg) const
{
  if(callback_log_) callback_log_(lvl, msg);
}

// -----------------------------------------------------------------------------

void ClientSettings::out_header(std::ostream & stream) const
{
  ap_.out_header(stream);
}

// -----------------------------------------------------------------------------

void ClientSettings::out_help(std::ostream & stream, std::string_view app) const
{
  ap_.out_help(stream, app);
}

// -----------------------------------------------------------------------------

} // namespace tftp


