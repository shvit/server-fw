/*
 * tftpComplex_test.cpp
 *
 *  Created on: 24 сент. 2021 г.
 *      Author: svv
 */


#include "../tftpClientSession.h"
#include "../tftpSrv.h"
#include "test.h"
#include "tftpComplex_test.h"

UNIT_TEST_SUITE_BEGIN(ComplexTest)

//------------------------------------------------------------------------------

const ClientCases cc{
  {{},{0U,0U,0U,0U,0U,0U,0U}, 0U},
//  {{"--blksize", "1000"},{0U,0U,0U,0U,0U,0U,0U}, 0U},
};

UNIT_TEST_CASE_BEGIN(main, "Test server with client")

TestServer ts;
TEST_CHECK_TRUE(ts.valid());

if(!ts.valid())
{
  std::cout << "Skip server-client complex tests (server init failed)" << std::endl;
}
else
{
  // Run server
  usleep(100000U);

  // Run sesions

  ClientsData cl_data;
  tftp::ArgParser ap_cl{tftp::constants::client_arg_settings};

  const char * tst_argv[100U];
  size_t tst_argc;

  for(size_t it_cl = 0U; it_cl < cc.size(); ++it_cl)
  {
    for(int it_dir=1; it_dir >= 0; --it_dir)
    {
      cl_data.emplace_back();

      filesystem::path pf_new{unit_tests::local_dir};
      pf_new /= unit_tests::gen_file_name(cl_data.size());

      std::string f_new{pf_new.string()};
      std::cout << " * Client file (" << cl_data.size() << ") " << f_new << " by " << (it_dir?"RRQ":"WRQ") << std::endl;

      tst_argc =0U;
      tst_argv[tst_argc++] = "tftp-client-fake";
      if(it_dir)
      { // RRQ
        tst_argv[tst_argc++] = "--get";
        tst_argv[tst_argc++] = "-r";
        tst_argv[tst_argc++] = ts.main_filename().c_str(); //tst_file_name.c_str();
        tst_argv[tst_argc++] = "-l";
        tst_argv[tst_argc++] = f_new.c_str();
      }
      else
      { // WRQ
        tst_argv[tst_argc++] = "--put";
        tst_argv[tst_argc++] = "-l";
        tst_argv[tst_argc++] = ts.main_filename().c_str(); //tst_file_name.c_str();
        tst_argv[tst_argc++] = "-r";
        tst_argv[tst_argc++] = f_new.c_str();
      }


      tst_argv[tst_argc++] = ts.listening().c_str(); //srv_addr.c_str();

    auto cb_syslog_clnt = std::bind(
        & unit_tests::FakeLog<7>::syslog,
        & cl_data[cl_data.size() - 1U].log,
        std::placeholders::_1,
        std::placeholders::_2);

    ap_cl.run(
        cb_syslog_clnt,
        tst_argc,
        const_cast<char **>(tst_argv));

    auto p_sett = tftp::ClientSettings::create();

    p_sett->load_options(cb_syslog_clnt, ap_cl);

    cl_data[cl_data.size() - 1U].p_ses = tftp::ClientSession::create(
        std::move(p_sett),
        cb_syslog_clnt);

    TEST_CHECK_TRUE(cl_data[cl_data.size() - 1U].p_ses->init());

    cl_data[cl_data.size() - 1U].thr = std::thread{
      & tftp::ClientSession::run_session,
      cl_data[cl_data.size() - 1U].p_ses.get()};

    }
  }

  std::cout << "All clients executed" << std::endl;

  // Wait all clients finished
  if(cl_data.size())
  {
    bool any_alive = true;
    while(any_alive)
    {
      any_alive = false;

      for(auto & cl : cl_data)
      {
        any_alive = any_alive || !cl.p_ses->is_finished();
      }
    }

    for(auto & cl : cl_data) cl.thr.join();
  }

  std::cout << "All clients finished" << std::endl;
}

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
