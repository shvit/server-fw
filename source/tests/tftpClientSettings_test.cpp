/*
 * tftpClientSettings_test.cpp
 *
 *  Created on: 9 июн. 2021 г.
 *      Author: svv
 */




#include "../tftpCommon.h"
#include "../tftpClientSettings.h"
#include "test.h"

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(ClientSettings)

//------------------------------------------------------------------------------

/** \brief Helper class for unit-test access to Settings protected field
 */
class Settings_test: public tftp::ClientSettings
{
public:
  Settings_test():
    ClientSettings()
  {
    callback_log_ = std::bind(
        & Settings_test::cout_log,
        this,
        std::placeholders::_1,
        std::placeholders::_2);

  };

  //using ClientSettings::ClientSettings;
  using ClientSettings::srv_addr_;
  using ClientSettings::verb_;
  using ClientSettings::file_local_;
  using ClientSettings::file_remote_;
  using ClientSettings::callback_log_;

  void cout_log(tftp::LogLvl lvl, std::string_view msg) const
  {
    std::cout << "[DEBUG] " << tftp::to_string(lvl) << " " <<  msg << std::endl;
  }
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(parse_arg_cl, "Parse CMD client arguments")

// 1
START_ITER("default options");
{
  Settings_test b;
  TEST_CHECK_TRUE (b.srv_addr_.family() == 0U);
  TEST_CHECK_TRUE (b.srv_addr_.port() == tftp::constants::default_tftp_port);
  TEST_CHECK_FALSE(b.verb_);
  TEST_CHECK_TRUE (b.file_local_.size() == 0U);
  TEST_CHECK_TRUE (b.file_remote_.size() == 0U);
  //TEST_CHECK_TRUE (b.callback_log_ == nullptr);

}

// 2
START_ITER("load options IPv4");
{
  /*
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
    "10.0.0.202:6900",
  };

  Settings_test b;
  TEST_CHECK_TRUE(b.load_options(sizeof(tst_args)/sizeof(tst_args[0]),
                                 const_cast<char **>(tst_args)));
  TEST_CHECK_TRUE(b.verb_);
  TEST_CHECK_TRUE(b.file_local_ == "test_local.txt");
  TEST_CHECK_TRUE(b.file_remote_ == "test_remote.txt");
  TEST_CHECK_TRUE(b.blksize() == 1300);
  TEST_CHECK_TRUE(b.timeout() == 20);
  TEST_CHECK_TRUE(b.windowsize() == 15);
  TEST_CHECK_TRUE(b.request_type() == tftp::SrvReq::read);
  TEST_CHECK_TRUE(b.transfer_mode() == tftp::TransfMode::netascii);


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
  TEST_CHECK_TRUE(b.file_chown_user == "usr1");
  TEST_CHECK_TRUE(b.file_chown_grp == "grp2");
  TEST_CHECK_TRUE(b.file_chmod == 0766);
  */
}

// 3
START_ITER("load options IPv6");
{
  /*
  const char * tst_args[]=
  {
    "./server-fw",
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
  */
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
