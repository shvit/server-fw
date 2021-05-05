/*
 * tftpBase_test.cpp
 *
 *  Created on: 13 апр. 2021 г.
 *      Author: svv
 */

#include <netinet/in.h>

#include "../tftpCommon.h"
#include "../tftpBase.h"
#include "../tftpSettings.h"
#include "test.h"

UNIT_TEST_SUITE_BEGIN(Base)


class tst_Base: public tftp::Base
{
public:
  using tftp::Base::settings_;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(main, "")

  tst_Base b;

START_ITER("Default values")
{
  //std::cout << " * '" << b.settings_->retransmit_count_ << "'" << std::endl;

  TEST_CHECK_TRUE(b.get_root_dir() == "");
  TEST_CHECK_TRUE(b.get_lib_name_fb() == tftp::constants::default_fb_lib_name);
  TEST_CHECK_FALSE(b.get_is_daemon());
  TEST_CHECK_TRUE(b.get_retransmit_count() == tftp::constants::default_retransmit_count);

  auto a4 = b.server_addr();

  TEST_CHECK_TRUE(a4.family() == AF_INET);
  TEST_CHECK_TRUE(a4.port() == tftp::constants::default_tftp_port);
  TEST_CHECK_TRUE(a4.as_in().sin_addr.s_addr == 0U);
  TEST_CHECK_TRUE(a4.str() == "0.0.0.0:"+std::to_string(tftp::constants::default_tftp_port));

  auto sd = b.get_serach_dir();
  TEST_CHECK_TRUE(sd.size() == 0U);
}

START_ITER("Modify values and try again")
{
  //
  b.settings_->root_dir = "/temp/server_dir";
  b.settings_->lib_name = "/bin/hrenak";
  b.settings_->local_base_.set_string("[fe80::1]:65012");
  b.settings_->is_daemon = true;
  b.settings_->retransmit_count_ = 3U*tftp::constants::default_retransmit_count;
  b.settings_->backup_dirs.push_back("/dir1");
  b.settings_->backup_dirs.push_back("/dir2/");
  b.settings_->backup_dirs.push_back("/dir3/");

  //std::cout << " * '" << b.settings_->retransmit_count_ << "'" << std::endl;

  TEST_CHECK_TRUE(b.get_root_dir() == "/temp/server_dir/");
  TEST_CHECK_TRUE(b.get_lib_name_fb() == "/bin/hrenak");
  TEST_CHECK_TRUE(b.get_is_daemon());
  TEST_CHECK_TRUE(b.get_retransmit_count() == 3U*tftp::constants::default_retransmit_count);

  auto a6 = b.server_addr();

  TEST_CHECK_TRUE(a6.family() == AF_INET6);
  TEST_CHECK_TRUE(a6.port() == 65012);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 0] == 0xfeU);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 1] == 0x80U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 2] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 3] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 4] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 5] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 6] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 7] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 8] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[ 9] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[10] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[11] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[12] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[13] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[14] == 0x00U);
  TEST_CHECK_TRUE(a6.as_in6().sin6_addr.__in6_u.__u6_addr8[15] == 0x01U);
  TEST_CHECK_TRUE(a6.str() == "[fe80::1]:65012");

  auto sd = b.get_serach_dir();
  TEST_CHECK_TRUE(sd.size() == 3U);
  TEST_CHECK_TRUE(sd[0U] == "/dir1");
  TEST_CHECK_TRUE(sd[1U] == "/dir2/");
  TEST_CHECK_TRUE(sd[2U] == "/dir3/");
}

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
