/**
 * \file tftpClientSettingss_test.cpp
 * \brief Unit-tests for class ClientSettingss
 *
 *  License GPL-3.0
 *
 *  \date 09-jun-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include "../tftpCommon.h"
#include "../tftpClientSettings.h"
#include "test.h"

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(ClientSettings)

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(parse_arg_cl, "Parse CMD client arguments")

size_t log_err=0U;
size_t log_wrn=0U;
size_t log_inf=0U;
size_t log_dbg=0U;

auto log_local=[&](tftp::LogLvl lvl, std::string_view msg)
{
  switch(lvl)
  {
    case tftp::LogLvl::err:     ++log_err; break;
    case tftp::LogLvl::warning: ++log_wrn; break;
    case tftp::LogLvl::info:    ++log_inf; break;
    case tftp::LogLvl::debug:   ++log_dbg; break;
    default:  break;
  }
  std::cout << "[DEBUG] " << tftp::to_string(lvl) << " " <<  msg << std::endl;
};

// 1
START_ITER("default options");
{
  auto b = tftp::ClientSettings::create();

  TEST_CHECK_TRUE(b->srv_addr.family() == AF_INET);
  TEST_CHECK_TRUE(b->srv_addr.port() == tftp::constants::default_tftp_port);
  TEST_CHECK_TRUE(b->verb == 4);
  TEST_CHECK_TRUE(b->file_local.size() == 0U);
  TEST_CHECK_TRUE(b->opt.filename().size() == 0U);
}

// 2
START_ITER("load options normal");
{
  log_err = 0U;
  log_wrn = 0U;
  log_inf = 0U;
  log_dbg = 0U;

  const char * tst_args[]=
  {
    "./tftp-cl",
    "-g",
    "-v",
    "-l", "test_local.txt",
    "-r", "test_remote.txt",
    "-m", "netascii",
    "-b", "1300",
    "-t", "20",
    "-w", "15",
    "--tsize", "15000001",
    "10.0.0.202:6900",
  };

  tftp::ArgParser ap{tftp::constants::client_arg_settings};

  ap.run(
      log_local,
      sizeof(tst_args)/sizeof(tst_args[0]),
      const_cast<char **>(tst_args));

  auto b = tftp::ClientSettings::create();

  TEST_CHECK_TRUE(b->load_options(log_local, ap) == tftp::TripleResult::ok);

  TEST_CHECK_TRUE(log_err == 0U);
  TEST_CHECK_TRUE(log_wrn == 2U);
  TEST_CHECK_TRUE(log_inf == 2U);
  TEST_CHECK_TRUE(log_dbg == 12U);

  TEST_CHECK_TRUE(b->verb == 7);
  TEST_CHECK_TRUE(b->file_local == "test_local.txt");
  TEST_CHECK_TRUE(b->opt.filename() == "test_remote.txt");
  TEST_CHECK_TRUE(b->opt.blksize() == 1300);
  TEST_CHECK_TRUE(b->opt.timeout() == 20);
  TEST_CHECK_TRUE(b->opt.windowsize() == 15);
  TEST_CHECK_TRUE(b->opt.tsize() == 15000001);
  TEST_CHECK_TRUE(b->opt.request_type() == tftp::SrvReq::read);
  TEST_CHECK_TRUE(b->opt.transfer_mode() == tftp::TransfMode::netascii);
  TEST_CHECK_TRUE(b->srv_addr.str() == "10.0.0.202:6900");
}

// 3
START_ITER("Try to load fail options");
{
  log_err = 0U;
  log_wrn = 0U;
  log_inf = 0U;
  log_dbg = 0U;

  const char * tst_args[]=
  {
    "./tftp-cl",
    "-m", "hren",
    "-b", "10000000000",
    "-t", "5555555555",
    "-w", "0",
    "--tsize", "awwdswd",
    "--verb", "5",
  };

  tftp::ArgParser ap{tftp::constants::client_arg_settings};

  ap.run(
      log_local,
      sizeof(tst_args)/sizeof(tst_args[0]),
      const_cast<char **>(tst_args));

  auto b = tftp::ClientSettings::create();

  TEST_CHECK_TRUE(b->load_options(log_local, ap) == tftp::TripleResult::ok);

  TEST_CHECK_TRUE (log_err == 0U);
  TEST_CHECK_TRUE (log_wrn == 6U);
  TEST_CHECK_TRUE (log_inf == 0U);
  TEST_CHECK_TRUE (log_dbg == 10U);

  TEST_CHECK_TRUE (b->opt.transfer_mode() == tftp::TransfMode::octet);
  TEST_CHECK_FALSE(b->opt.was_set_blksize());
  TEST_CHECK_FALSE(b->opt.was_set_timeout());
  TEST_CHECK_FALSE(b->opt.was_set_windowsize());
  TEST_CHECK_FALSE(b->opt.was_set_tsize());
  TEST_CHECK_TRUE (b->srv_addr.str() == "127.0.0.1:69");
  TEST_CHECK_TRUE (b->verb == 5);
}

// 4 - real bug case
{
  const char * tst_args[]=
  {
    "./tftp-cl",
    "--get",
    "-l", "z",
    "-r", "z",
    "10.0.0.202:69",
    "-v", "7",
  };

  tftp::ArgParser ap{tftp::constants::client_arg_settings};

  ap.run(
      log_local,
      sizeof(tst_args)/sizeof(tst_args[0]),
      const_cast<char **>(tst_args));

  auto b = tftp::ClientSettings::create();

  TEST_CHECK_TRUE(b->load_options(log_local, ap) == tftp::TripleResult::ok);

  TEST_CHECK_TRUE (log_err == 0U);
  TEST_CHECK_TRUE (log_wrn == 0U);

  TEST_CHECK_TRUE(b->opt.request_type() == tftp::SrvReq::read);

}


UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
