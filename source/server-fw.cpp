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
#include <sys/syscall.h>
#include <thread>

#include <iostream>
#include <unistd.h>

#include "tftpCommon.h"
#include "tftpSrv.h"

int main(int argc, char* argv[])
{
  using RuntimeSrv = std::pair<tftp::pSrv, std::thread>;
  using RuntimeSrvs = std::list<RuntimeSrv>;

  tftp::LogLines temp_log;

  bool arg_finish=false;
  tftp::LogLvl curr_verb = tftp::LogLvl::debug;
  bool         curr_daemon = false;

  auto log_main=[&](const tftp::LogLvl lvl, std::string_view msg)
    {
      if(arg_finish)
      {
        if(lvl <= curr_verb)
        {
          if(curr_daemon)
          {
            syslog((int)lvl,
                      "[%d] %s %s",
                      (int)syscall(SYS_gettid),
                      to_string(lvl).data(),
                      msg.data());
          }
          else
          {
            std::cout << "[" << std::to_string((int)syscall(SYS_gettid)) << "] "
                      << tftp::to_string(lvl) << " " << msg << std::endl;
          }
        }
      }
      else
      {
        temp_log.emplace_back(lvl, msg);
      }
    };

#define CURR_LOG(L,M) log_main(tftp::LogLvl::L,"tftp::"+std::string{tftp::constants::app_srv_name}+"::main() "+M)

  CURR_LOG(debug, "Begin");

  tftp::ArgParser ap{tftp::constants::srv_arg_settings};

  auto log_pre_out=[&]()
  {
    if(arg_finish) return;
    if(!curr_daemon) ap.out_header(std::cout);

    if(curr_daemon)
      openlog(tftp::constants::app_srv_name.data(), LOG_NDELAY, LOG_DAEMON); // LOG_PID

    arg_finish=true;
    for(const auto & item : temp_log) log_main(item.first, item.second);
    temp_log.clear();
  };

  ap.run(log_main, argc, argv);

  tftp::SrvSettings srv_st;
  auto res_apply = srv_st.load_options(log_main, ap);

  curr_verb = srv_st.get_verb();
  curr_daemon = srv_st.get_is_daemon();

  // Check need exit if no continue
  switch(res_apply)
  {
    case tftp::TripleResult::fail:
    {
      CURR_LOG(err, "Fail load server arguments");
      log_pre_out();
      return EXIT_FAILURE;
    }
    case tftp::TripleResult::nop:
      ap.out_help(std::cout, tftp::constants::app_srv_name);
      return EXIT_SUCCESS;
    default: // go next
      break;
  }

  // Out header and backuped logging messages (and clear temp storage)
  log_pre_out();

  // Fork if run as daemon
  if(curr_daemon)
  {
    auto pid = fork();

    if(pid<0)
    {
      CURR_LOG(err, "Daemon start failed (fork error)");
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
          CURR_LOG(err, "Failed use chdir(\"/\")");
        }
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        CURR_LOG(info, "Run as daemon");
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

        return EXIT_SUCCESS;
      }
    }
  }

  // Iter over all listening address and run threads
  RuntimeSrvs srvs;
  for(const auto & la : ap.result().second)
  {
    CURR_LOG(debug, "Try listening '"+la+"'");

    auto news_srv = tftp::Srv::create(log_main, srv_st);

    if(news_srv->init(la))
    {
      auto bakup_ptr = news_srv.get();

      srvs.emplace_back(
          RuntimeSrv{
              std::move(news_srv),
              std::thread{& tftp::Srv::main_loop, bakup_ptr}});
    }
    else
    {
      CURR_LOG(debug, "Skip listening '"+la+"'");
    }
  }

  // Main loop - do while live threads
  while(srvs.begin() != srvs.end())
  {
    for(auto it = srvs.begin(); it != srvs.end(); ++it)
    {
      if(it->first->is_stopped())
      {
        CURR_LOG(debug, "Kill resource thread 1/"+std::to_string(srvs.size()));
        it->second.join();
        srvs.erase(it);
        break;
      }
    }

    usleep(100000);

    // DEBUG! Stop all threads after 5 sec
    //usleep(5000000); for(auto it = srvs.begin(); it != srvs.end(); ++it) it->first->stop();

  }

  CURR_LOG(debug, "End normal");
  if(curr_daemon) closelog();

  if(!curr_daemon) std::cout << "EXIT" << std::endl;
  return EXIT_SUCCESS;

}
