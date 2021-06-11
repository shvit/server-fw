/*
 * tftpClientSettings.cpp
 *
 *  Created on: 9 июн. 2021 г.
 *      Author: svv
 */

#include <getopt.h>

#include "tftpClientSettings.h"

namespace tftp
{

// -----------------------------------------------------------------------------

ClientSettings::ClientSettings():
    Options(),
    srv_addr_{},
    verb_{false},
    file_local_{},
    file_remote_{},
    callback_log_{nullptr}
{
  srv_addr_.set_port(constants::default_tftp_port);
}

// -----------------------------------------------------------------------------

ClientSettings::~ClientSettings()
{
}

// -----------------------------------------------------------------------------

auto ClientSettings::create() -> pClientSettings
{
  return std::make_unique<ClientSettings>(ClientSettings{});
}

//------------------------------------------------------------------------------

bool ClientSettings::load_options(int argc, char * argv[])
{
  L_DBG("Start argument parse ("+std::to_string(argc)+")");

  bool ret = true;
/*
  static const char *optString = "l:L:r:R:gGpPhHvVm:M:b:B:w:W:t:T:";

  static const struct option longOpts[] =
  {
      { "local",      required_argument, NULL, 'l' }, // 0
      { "remote",     required_argument, NULL, 'r' }, // 1
      { "get",              no_argument, NULL, 'g' }, // 2
      { "put",              no_argument, NULL, 'p' }, // 3
      { "help",             no_argument, NULL, 'h' }, // 4
      { "verbose",          no_argument, NULL, 'v' }, // 5
      { "mode",       required_argument, NULL, 'm' }, // 6
      { "blksize",    required_argument, NULL, 'b' }, // 7
      { "windowsize", required_argument, NULL, 'w' }, // 8
      { "timeout",    required_argument, NULL, 't' }, // 9
      { "tsize",      optional_argument, NULL,  0  }, // 10
      { NULL,               no_argument, NULL,  0  }  // always last
  };

  optind=1;
  while(argc > 1)
  {
    if(optind<argc)
    {
      L_DBG("optind "+std::to_string(optind)+" '"+std::string{argv[optind]}+"'");
      if(argv[optind][0]!='-') L_DBG("   FIND FREE VALUE: "+std::string{argv[optind]});
    }
    int longIndex;
    int opt = getopt_long(argc, argv, optString, longOpts, & longIndex);
    if(opt == -1) break; // end parsing

    L_DBG("   go opt "+std::to_string(opt) + " '"+(char)opt+"'");
    L_DBG("   next optind "+std::to_string(optind)+" '"+std::string{argv[optind]}+"'");

    switch(opt)
    {
      case 'l':
      case 'L':
        if(optarg) file_local_.assign(optarg);
        break;
      case 'r':
      case 'R':
        if(optarg) file_remote_.assign(optarg);
        break;
      case 'g':
      case 'G':
        request_type_ = SrvReq::read;
        break;
      case 'p':
      case 'P':
        request_type_ = SrvReq::write;
        break;
    case 'h':
    case 'H':
    case '?':
      ret = false; // Help message cout
      break;
    case 'v':
    case 'V':
      verb_ = true;
      break;
    case 'm':
    case 'M':
      if(optarg)
      {
        std::string temp_str{optarg};
        do_lower(temp_str);
        if(temp_str == to_string(TransfMode::octet)) transfer_mode_ = TransfMode::octet;
        else
        if(temp_str == to_string(TransfMode::mail)) transfer_mode_ = TransfMode::mail;
        else
        if(temp_str == to_string(TransfMode::netascii)) transfer_mode_ = TransfMode::netascii;
        else
        if(temp_str == to_string(TransfMode::binary)) transfer_mode_ = TransfMode::binary;
        else
        transfer_mode_ = TransfMode::unknown;
      }
      break;

    case 'b':
    case 'B':
      if(optarg)
      {
        int tmp_int = std::stol(optarg);
        if(tmp_int > 0)
        {
          blksize_ = {true, tmp_int};
        }
      }
      break;
    case 'w':
    case 'W':
      if(optarg)
      {
        int tmp_int = std::stol(optarg);
        if(tmp_int > 0)
        {
          windowsize_ = {true, tmp_int};
        }
      }
      break;
    case 't':
    case 'T':
      if(optarg)
      {
        int tmp_int = std::stol(optarg);
        if(tmp_int > 0)
        {
          timeout_ = {true, tmp_int};
        }
      }
      break;

    case 0:
      switch(longIndex)
      {
      case 10: // --tsize
        if(optarg)
        {
          int tmp_int = std::stol(optarg);
          if(tmp_int > 0)
          {
            tsize_ = {true, tmp_int};
          }
          else
          { // only for WRQ use option with fill real file size (ignore arg value)
            tsize_ = {true, 0};
          }
        }
        break;

      default:
        L_ERR("Wrong long option index "+std::to_string(longIndex));
        break;

      } // switch for long options
      break;

    default:
      L_ERR("Wrong short option index "+std::to_string(opt));
      break;

    } // switch for short options
  }
*/
  L_DBG("Finish argument parse is "+(ret ? "SUCCESS" : "FAIL"));

  return ret;
}

// -----------------------------------------------------------------------------

void ClientSettings::log(LogLvl lvl, std::string_view msg) const
{
  if(callback_log_) callback_log_(lvl, msg);
}

// -----------------------------------------------------------------------------


} // namespace tftp


