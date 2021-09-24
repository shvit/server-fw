/*
 * tftpComplex_test.cpp
 *
 *  Created on: 24 сент. 2021 г.
 *      Author: svv
 */


#include "../tftpClientSession.h"
#include "../tftpSrv.h"
#include "test.h"

UNIT_TEST_SUITE_BEGIN(ComplexTest)

//------------------------------------------------------------------------------

using LogVector = std::array<size_t, 7U>;

/** \brief Settings for one client case
 *  1 - Client arguments
 *  2 - result logging with FakeLog
 *  3 - Correct file data size
 *  Warn: local and remote filenames generated automatic by test case
 *
 */
using ClientCase = std::tuple<tftp::VecStr, LogVector, size_t>;

using ClientCases = std::vector<ClientCase>;

const ClientCases cc{
  {{},{0U,0U,0U,0U,0U,0U,0U}, 0U},
  {{"--blksize", "1000"},{0U,0U,0U,0U,0U,0U,0U}, 0U},
};

UNIT_TEST_CASE_BEGIN(main, "Test server with client")

// Prepare

TEST_CHECK_TRUE(unit_tests::check_local_directory());
//std::cout << "* Local dir " << unit_tests::local_dir << std::endl;
//std::cout << "* Curr path " << filesystem::current_path() << std::endl;

// Source filename
filesystem::path src_file{filesystem::current_path()};
src_file /= "bin";
src_file /=  tftp::constants::app_srv_name;
TEST_CHECK_TRUE(filesystem::exists(src_file));
//std::cout << "* Src file " << src_file << std::endl;

// Destination main test filename
std::string tst_file_name{unit_tests::gen_file_name(0U)};
filesystem::path tst_file{unit_tests::local_dir};
tst_file /= tst_file_name;
//std::cout << "* Dst file " << tst_file << std::endl;

// Copy test file
TEST_CHECK_TRUE(filesystem::copy_file(src_file, tst_file));
TEST_CHECK_TRUE(filesystem::exists(tst_file));

// Gen server port for listening
uint16_t srv_port = unit_tests::gen_test_port();
TEST_CHECK_TRUE((srv_port >= unit_tests::port_min) && (srv_port <= unit_tests::port_max));
//std::cout << "* Srv port " << srv_port << std::endl;

std::string srv_addr{"0.0.0.0:"};
srv_addr.append(std::to_string(srv_port));
std::cout << "* Srv listening " << srv_addr << std::endl;

// Logger for server
unit_tests::FakeLog<7> fl_srv;
auto cb_syslog_srv = std::bind(
    & unit_tests::FakeLog<7>::syslog,
    & fl_srv,
    std::placeholders::_1,
    std::placeholders::_2);

unit_tests::FakeLog<7> fl_clnt;
auto cb_syslog_clnt = std::bind(
    & unit_tests::FakeLog<7>::syslog,
    & fl_clnt,
    std::placeholders::_1,
    std::placeholders::_2);


//unit_tests::FakeError fe;
//auto cb_seterr = std::bind(
    //& unit_tests::FakeError::set_error,
    //& fe,
    //std::placeholders::_1,
    //std::placeholders::_2);

const char * srv_args[]=
{
  "./server-fw",
  "--root-dir", unit_tests::local_dir.c_str(),
  "--daemon",
  "--syslog", "7",
  "--file-chuser", "usr",
  "--file-chgrp", "grp",
  "--file-chmod", "0666",
  srv_addr.c_str()
};

tftp::ArgParser ap_srv{tftp::constants::srv_arg_settings};

ap_srv.run(
    cb_syslog_srv,
    sizeof(srv_args)/sizeof(srv_args[0]),
    const_cast<char **>(srv_args));

TEST_CHECK_TRUE(fl_srv.chk_clear({0,0,0,0,0,0,9}));

tftp::SrvSettings srv_st;


TEST_CHECK_TRUE(srv_st.load_options(cb_syslog_srv, ap_srv) == tftp::TripleResult::ok);

TEST_CHECK_TRUE(fl_srv.chk_clear({0,0,0,0,0,0,2}));

auto tst_srv = tftp::Srv::create(cb_syslog_srv, srv_st);

bool is_srv_init=false;
TEST_CHECK_TRUE((is_srv_init = tst_srv->init(srv_addr)));

TEST_CHECK_TRUE(fl_srv.chk_clear({0,0,0,0,0,3,0}));

if(!is_srv_init)
{
  std::cout << "Skip server-client complex tests (server init failed)" << std::endl;
  goto FINISH;
}

{ // server OK

  auto srv_thr = std::thread{& tftp::Srv::main_loop, tst_srv.get()};
  usleep(500000U);

  const char * tst_argv[100U];
  size_t tst_argc;
  size_t it_case=0U;

  for(size_t it_cl = 0U; it_cl < cc.size(); ++it_cl)
  {
    for(int it_dir=1; it_dir >= 0; --it_dir)
    {
      ++it_case;
      std::string f_new{unit_tests::gen_file_name(it_case)};

      tst_argc =0U;
      tst_argv[tst_argc++] = "tftp-client-fake";
      if(it_dir)
      { // RRQ
        tst_argv[tst_argc++] = "--get";
        tst_argv[tst_argc++] = "-r";
        tst_argv[tst_argc++] = tst_file_name.c_str();
        tst_argv[tst_argc++] = "-l";
        tst_argv[tst_argc++] = f_new.c_str();
      }
      else
      { // WRQ
        tst_argv[tst_argc++] = "--put";
        tst_argv[tst_argc++] = "-l";
        tst_argv[tst_argc++] = tst_file_name.c_str();
        tst_argv[tst_argc++] = "-r";
        tst_argv[tst_argc++] = f_new.c_str();
      }


      tst_argv[tst_argc++] = srv_addr.c_str();
    }


  }

//fl_srv.show();
//fl_srv.verb_on();

tftp::ArgParser ap_clnt{tftp::constants::client_arg_settings};

ap_clnt.run(cb_syslog_clnt, 0, nullptr);



std::cout << "* 1" << std::endl;

usleep(15000000U);

std::cout << "* 2" << std::endl;


//FIN_THR: // terminate server

tst_srv->stop();
while(!tst_srv->is_stopped()) {};
srv_thr.join();
}

FINISH:

// delete temporary files
unit_tests::files_delete();

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
