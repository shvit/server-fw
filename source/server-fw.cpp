/**
 * \file server-fw.cpp
 * \brief TFTP server application
 *
 *  TFTP server application
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <iostream>
#include <unistd.h>

#include "tftpCommon.h"
#include "tftpSrv.h"

int main(int argc, char* argv[])
{
  using LogLine = std::pair<tftp::LogLvl, std::string>;
  using LogLines = std::vector<LogLine>;

  LogLines temp_log;

  auto log_pre=[&](const tftp::LogLvl l, std::string_view m)
  {
    temp_log.emplace_back(l, m);
  };

  auto log_pre_out=[&](std::ostream & stream, tftp::LogLvl lvl = tftp::LogLvl::debug)
  {
    for(const auto & item : temp_log)
    {
      if(item.first <= lvl)
        stream << "[" << tftp::to_string(item.first) << "] " << item.second << std::endl;
    }
  };

  tftp::ArgParser ap{tftp::constants::srv_arg_settings};

  ap.run(argc, argv);

  auto ss = tftp::SrvSettingsStor::create();

  switch(ss->load_options(log_pre, ap))
  {
    case tftp::TripleResult::fail:
    {
      ss->out_id(std::cout);
      //log_pre(tftp::LogLvl::err, "Fail load server arguments");
      log_pre_out(std::cerr, ((ss->verb >=0 && (ss->verb <=7)) ? (tftp::LogLvl)ss->verb : tftp::LogLvl::debug));
      return EXIT_FAILURE;
    }
    case tftp::TripleResult::nop:
      ss->out_help(std::cout, tftp::constants::app_srv_name);
      return EXIT_SUCCESS;
    default: // go next
      break;
  }

  if(!ss->is_daemon)
  {
    ss->out_id(std::cout);
    log_pre_out(std::cout, ((ss->verb >=0 && (ss->verb <=7)) ? (tftp::LogLvl)ss->verb : tftp::LogLvl::debug));
  }


  std::cout << "EXIT" << std::endl;
  return EXIT_SUCCESS;








  constexpr const int fake_exit_code=1000;  // fake value

  int exit_code = fake_exit_code;

  openlog(tftp::constants::app_srv_name.data(), LOG_NDELAY, LOG_DAEMON); // LOG_PID

  tftp::Srv server;

  if(server.load_options(nullptr, argc, argv))
  {
    if(server.get_is_daemon())
    {
      pid_t pid = fork();

      if(pid<0)
      {
        server.log(tftp::LogLvl::err, "Daemon start failed (fork error)");
        return EXIT_FAILURE;
      }
      else
      {
        if(!pid)
        { // daemon code
          umask(0);
          setsid();
          if(auto ret=chdir("/"); ret != 0)
          {
            server.log(tftp::LogLvl::err, "Failed use chdir(\"/\")");
          }
          close(STDIN_FILENO);
          close(STDOUT_FILENO);
          close(STDERR_FILENO);
          server.log(tftp::LogLvl::info, "Run as daemon");
        }
        else
        { // app finalize code
            std::cout << "Daemon (" << pid << ") start ... ";
          pid_t    wp;
          uint32_t wc=0;
          while ( (wp = waitpid(pid, NULL, WNOHANG)) == 0 )
          { // wait some sec (daemon can closed)
            usleep(50000);
            wc++;
            if(wc>25) break;
          }

          if(wp) std::cout << "Failed" << std::endl <<"Daemon not started; see syslog for detail" << std::endl;
            else std::cout << "Successfull" << std::endl;

          exit_code = EXIT_SUCCESS;
        }
      }
    }
    else
    {
      server.out_id(std::cout);
    }

    if(exit_code == fake_exit_code)
    {
      if(server.init())
      {
        server.main_loop();
      }
      else
      {
        std::cout << "Fail server init" << std::endl;
        exit_code = EXIT_FAILURE;
      }
    }
  }
  else
  {
    // Fail load options
    server.out_help(std::cout, argv[0]);
    exit_code = EXIT_FAILURE;
  }

  closelog();

  if(exit_code == fake_exit_code) exit_code = EXIT_SUCCESS;

  return exit_code;
}
