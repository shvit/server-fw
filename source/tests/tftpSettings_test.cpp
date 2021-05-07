/**
 * \file tftpSettings_test.cpp
 * \brief Unit-tests for class Settings
 *
 *  License GPL-3.0
 *
 *  \date   07-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include "../tftpCommon.h"
#include "../tftpSettings.h"
#include "test.h"

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(Settings)

//------------------------------------------------------------------------------

/** \brief Helper class for unit-test access to Settings protected field
 */
class Settings_test: public tftp::Settings
{
public:
  using Settings::Settings;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(parse_arg, "Parse CMD arguments")

// 1
START_ITER("default options");
{
  Settings_test b;
  TEST_CHECK_FALSE(b.is_daemon);
  TEST_CHECK_TRUE(b.use_syslog == tftp::constants::default_tftp_syslog_lvl);
  TEST_CHECK_TRUE(b.local_base_.family() == AF_INET);
  TEST_CHECK_TRUE(b.local_base_.port() == tftp::constants::default_tftp_port);
  TEST_CHECK_TRUE(b.local_base_.as_in().sin_addr.s_addr == 0U);
  TEST_CHECK_TRUE(b.local_base_.str() == "0.0.0.0:69");
  TEST_CHECK_TRUE(& ((struct sockaddr_in *) b.local_base_.data())->sin_addr.s_addr ==
                  & b.local_base_.as_in().sin_addr.s_addr);

  //TEST_CHECK_TRUE(b.lib_dir == "/usr/lib/x86_64-linux-gnu/");
  //TEST_CHECK_TRUE(b.lib_name == "libfbclient.so");
  TEST_CHECK_TRUE(b.root_dir == "");
  TEST_CHECK_TRUE(b.backup_dirs.size() == 0);
  TEST_CHECK_TRUE(b.db == "");
  TEST_CHECK_TRUE(b.user == "");
  TEST_CHECK_TRUE(b.pass == "");
  TEST_CHECK_TRUE(b.role == "");
  TEST_CHECK_TRUE(b.dialect == 3U);
  TEST_CHECK_TRUE(b.retransmit_count_ == tftp::constants::default_retransmit_count);
}

// 2
START_ITER("load options IPv4");
{
  const char * tst_args[]=
  {
    "./server_fw",
    "--daemon",
    "--syslog", "7",
    "--ip", "1.1.1.1:7777",
    "--root-dir", "/mnt/tftp",
    "--search", "/mnt/tst1",
    "--search", "/mnt/tst2",
    "--search", "/mnt/tst3",
    "--fb-db", "tester.fdb",
    "--fb-user", "SYSDBA",
    "--fb-pass", "masterkey",
    "--fb-role", "none",
    "--fb-dialect", "3",
    "--lib-dir", "/tmp/libs",
    "--lib-name", "fbclient",
    "--retransmit", "59"
  };

  Settings_test b;
  b.load_options(sizeof(tst_args)/sizeof(tst_args[0]),
                 const_cast<char **>(tst_args));
  TEST_CHECK_TRUE(b.is_daemon);
  TEST_CHECK_TRUE(b.use_syslog == 7);
  TEST_CHECK_TRUE(b.local_base_.family() == 2U);
  TEST_CHECK_TRUE(b.local_base_.port() == 7777U);
  TEST_CHECK_TRUE(b.local_base_.as_in().sin_addr.s_addr == 16843009U);
  TEST_CHECK_TRUE(b.local_base_.str() == "1.1.1.1:7777");
  TEST_CHECK_TRUE(b.lib_dir == "/tmp/libs");
  TEST_CHECK_TRUE(b.lib_name == "fbclient");
  TEST_CHECK_TRUE(b.root_dir == "/mnt/tftp");
  TEST_CHECK_TRUE(b.backup_dirs.size() == 3);
  TEST_CHECK_TRUE(b.backup_dirs[0] == "/mnt/tst1");
  TEST_CHECK_TRUE(b.backup_dirs[1] == "/mnt/tst2");
  TEST_CHECK_TRUE(b.backup_dirs[2] == "/mnt/tst3");
  TEST_CHECK_TRUE(b.db == "tester.fdb");
  TEST_CHECK_TRUE(b.user == "SYSDBA");
  TEST_CHECK_TRUE(b.pass == "masterkey");
  TEST_CHECK_TRUE(b.role == "none");
  TEST_CHECK_TRUE(b.dialect == 3U);
  TEST_CHECK_TRUE(b.retransmit_count_ == 59U);
}

// 3
START_ITER("load options IPv6");
{
  const char * tst_args[]=
  {
    "./server_fw",
    "--ip", "[fe80::1]:65000",
    "--root-dir", "/mnt/tftp",
  };

  Settings_test b;
  b.load_options(sizeof(tst_args)/sizeof(tst_args[0]),
                 const_cast<char **>(tst_args));
  TEST_CHECK_FALSE(b.is_daemon);
  TEST_CHECK_TRUE(b.use_syslog == tftp::constants::default_tftp_syslog_lvl);
  TEST_CHECK_TRUE(b.local_base_.family() == AF_INET6);
  TEST_CHECK_TRUE(b.local_base_.port() == 65000U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 0] == 0xfeU);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 1] == 0x80U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 2] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 3] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 4] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 5] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 6] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 7] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 8] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[ 9] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[10] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[11] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[12] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[13] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[14] == 0x00U);
  TEST_CHECK_TRUE(b.local_base_.as_in6().sin6_addr.__in6_u.__u6_addr8[15] == 0x01U);
  TEST_CHECK_TRUE(b.local_base_.str() == "[fe80::1]:65000");
  TEST_CHECK_TRUE(b.retransmit_count_ == tftp::constants::default_retransmit_count);
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
