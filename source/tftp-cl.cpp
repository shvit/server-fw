/*
 * tftp-cl.cpp
 *
 *  Created on: 23 июн. 2021 г.
 *      Author: svv
 */

//#include <future>
#include <iostream>
#include <string>

//#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>


#include "tftpClientSettings.h"
#include "tftpClientSession.h"

int main(int argc, char* argv[])
{
  tftp::LogLines temp_log;
  bool arg_finish=false;
  tftp::LogLvl curr_verb = tftp::LogLvl::warning;

  auto log_main=[&](const tftp::LogLvl lvl, std::string_view msg)
    {
      if(arg_finish)
      {
        if(lvl <= curr_verb)
        {
          std::cout << "[" << std::to_string((int)syscall(SYS_gettid)) << "] "
                    << tftp::to_string(lvl) << " " << msg << std::endl;
        }
      }
      else
      {
        temp_log.emplace_back(lvl, msg);
      }
    };

  tftp::ArgParser ap{tftp::constants::client_arg_settings};

  auto log_pre_out=[&]()
  {
    if(arg_finish) return;
    ap.out_header(std::cout);

    arg_finish=true;
    for(const auto & item : temp_log) log_main(item.first, item.second);
    temp_log.clear();
  };

  ap.run(log_main, argc, argv);

  auto cl_sett = tftp::ClientSettings::create();

  auto res_apply = cl_sett->load_options(log_main, ap);

  curr_verb = (tftp::LogLvl)cl_sett->verb;

  switch(res_apply)
  {
    case tftp::TripleResult::fail:
    {
      log_main(tftp::LogLvl::err, "Fail load client settings");
      log_pre_out();
      return EXIT_FAILURE;
    }
    case tftp::TripleResult::nop:
      ap.out_help(std::cout, "tftp-cl");
      return EXIT_SUCCESS;
    default: // go next
      break;
  }

  log_pre_out();

  tftp::ClientSession s(std::move(cl_sett), log_main);

  if(auto ret = s.run();
     ret != tftp::ClientSessionResult::ok) return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
