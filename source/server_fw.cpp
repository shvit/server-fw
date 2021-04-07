/**
 * \file server_fw.cpp
 * \brief TFTP server application
 *
 *  TFTP server application
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "tftpCommon.h"
#include "tftpSrv.h"

int main(int argc, char* argv[])
{
  constexpr const int fake_exit_code=1000;  // fake value

  int exit_code = fake_exit_code;

  [[maybe_unused]]
  auto logger = [](const tftp::LogLvl level, std::string_view message)
    {
      if((int)level <= LOG_DEBUG)
      {
        std::cout << "<" << tftp::to_string(level) << "> " << message << std::endl; // << std::flush();
      }
    };

  openlog("server_fw", LOG_NDELAY, LOG_DAEMON); // LOG_PID

  tftp::Srv server;

  if(server.load_options(argc, argv))
  {
    if(server.get_is_daemon())
    {
      pid_t pid = fork();

      if(pid<0)
      {
        syslog(LOG_ERR, "Daemon start failed (fork error)");
        return EXIT_FAILURE;
      }
      else
      {
        if(!pid)
        { // daemon code
          umask(0664);
          setsid();
          chdir("/");
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
      //server.set_logger(logger);
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
