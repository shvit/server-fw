/**
 * \file tftpSrvSession_test.cpp
 * \brief Unit-tests for class SrvSession
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <netinet/in.h>

#include "../tftpCommon.h"
#include "../tftpSrvSession.h"
#include "test.h"     

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(SrvSession)

//------------------------------------------------------------------------------

/** \brief Helper class for access to SrvSession protected field
 *
 */
class SrvSession_test: public tftp::SrvSession
{
public:
  SrvSession_test():
    tftp::SrvSession(tftp::SrvSettings(tftp::SrvSettingsStor::create()),
                  tftp::Logger()) {};

  using tftp::SrvSession::opt_;
  using tftp::SrvSession::stage_;
  using tftp::SrvSession::cl_addr_;
  using tftp::SrvSession::was_error;
  using tftp::SrvSession::set_error_if_first;
  using tftp::SrvSession::is_window_close;
  using tftp::SrvSession::step_back_window;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sess_init, "check prepare()")

  tftp::Addr b_addr;

  b_addr.set_family(AF_INET);
  b_addr.set_port(0x0506);
  *(b_addr.data() + 4U) = 1;
  *(b_addr.data() + 5U) = 2;
  *(b_addr.data() + 6U) = 3;
  *(b_addr.data() + 7U) = 4;

  tftp::SmBuf b_pkt
  {
    0,3, // fake type
    'f','i','l','e','n','a','m','e','.','t','x','t',0,
    'o','c','t','e','t',0,
    'b','l','k','s','i','z','e',0,'1','0','2','4',0,
    't','i','m','e','o','u','t',0,'1','0',0,
    't','s','i','z','e',0,'2','0','0','0','1','2','3',0
  };

  SrvSession_test s1;

  TEST_CHECK_FALSE(s1.prepare(b_addr, b_pkt, b_pkt.size()));
  TEST_CHECK_TRUE(s1.opt_.request_type() == tftp::SrvReq::unknown);

  b_pkt.set_be(0U, (int16_t)tftp::SrvReq::write);
  TEST_CHECK_TRUE (s1.prepare(b_addr, b_pkt, b_pkt.size()));

  TEST_CHECK_TRUE(std::equal(s1.cl_addr_.data(),
                             s1.cl_addr_.data() + b_addr.size(),
                             b_addr.cbegin()));
  TEST_CHECK_TRUE(s1.opt_.request_type() == tftp::SrvReq::write);
  TEST_CHECK_TRUE(s1.opt_.filename() == "filename.txt");
  TEST_CHECK_TRUE(s1.opt_.transfer_mode() == tftp::TransfMode::octet);

  TEST_CHECK_TRUE(s1.opt_.blksize() == 1024);
  TEST_CHECK_TRUE(s1.opt_.timeout() == 10);
  TEST_CHECK_TRUE(s1.opt_.tsize()   == 2000123);

  TEST_CHECK_TRUE(s1.opt_.was_set_blksize());
  TEST_CHECK_TRUE(s1.opt_.was_set_timeout());
  TEST_CHECK_TRUE(s1.opt_.was_set_tsize());

  TEST_CHECK_FALSE(s1.was_error());
  s1.set_error_if_first(909U, "Test error");
  TEST_CHECK_TRUE(s1.was_error());

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sess_win, "check windows")

START_ITER("windowsize==1")
{
  SrvSession_test s1;
  size_t stage = 0U;
  TEST_CHECK_TRUE(s1.opt_.windowsize() == 1U);
  TEST_CHECK_TRUE(s1.is_window_close(stage));
  TEST_CHECK_TRUE(s1.is_window_close(++stage));
  TEST_CHECK_TRUE(s1.is_window_close(++stage));
  TEST_CHECK_TRUE(s1.is_window_close(++stage));
  TEST_CHECK_TRUE(s1.is_window_close(++stage));
}

START_ITER("windowsize==5")
{
  SrvSession_test s1;

  tftp::Addr b_addr;
  b_addr.set_family(AF_INET);
  b_addr.set_port(0x0506);
  *(b_addr.data() + 4U) = 1;
  *(b_addr.data() + 5U) = 2;
  *(b_addr.data() + 6U) = 3;
  *(b_addr.data() + 7U) = 4;
  tftp::SmBuf b_pkt
  {
    0,1,
    'a',0,
    'o','c','t','e','t',0,
    'w','i','n','d','o','w','s','i','z','e',0,'5',0
  };
  s1.prepare(b_addr, b_pkt, b_pkt.size());
  size_t stage = 0U;
  TEST_CHECK_TRUE (s1.is_window_close(  stage));
  TEST_CHECK_FALSE(s1.is_window_close(++stage));
  TEST_CHECK_FALSE(s1.is_window_close(++stage));
  TEST_CHECK_FALSE(s1.is_window_close(++stage));
  TEST_CHECK_FALSE(s1.is_window_close(++stage));
  TEST_CHECK_TRUE (s1.is_window_close(++stage));
}

START_ITER("step_back_window()")
{
  SrvSession_test s1;

  tftp::Addr b_addr;
  b_addr.set_family(AF_INET);
  tftp::SmBuf b_pkt
  {
    0,1,
    'a',0,
    'o','c','t','e','t',0,
    'w','i','n','d','o','w','s','i','z','e',0,'5',0
  };
  s1.prepare(b_addr, b_pkt, b_pkt.size());
  size_t stage;

  s1.step_back_window(stage =     0U); TEST_CHECK_TRUE(stage ==     0U);
  s1.step_back_window(stage =     1U); TEST_CHECK_TRUE(stage ==     1U);
  s1.step_back_window(stage =     2U); TEST_CHECK_TRUE(stage ==     1U);
  s1.step_back_window(stage =     3U); TEST_CHECK_TRUE(stage ==     1U);
  s1.step_back_window(stage =     4U); TEST_CHECK_TRUE(stage ==     1U);
  s1.step_back_window(stage =     5U); TEST_CHECK_TRUE(stage ==     5U);
  s1.step_back_window(stage =     6U); TEST_CHECK_TRUE(stage ==     5U);
  s1.step_back_window(stage =     7U); TEST_CHECK_TRUE(stage ==     5U);
  s1.step_back_window(stage =     8U); TEST_CHECK_TRUE(stage ==     5U);
  s1.step_back_window(stage =     9U); TEST_CHECK_TRUE(stage ==     5U);
  s1.step_back_window(stage =    10U); TEST_CHECK_TRUE(stage ==    10U);
  s1.step_back_window(stage =    11U); TEST_CHECK_TRUE(stage ==    10U);
  s1.step_back_window(stage = 65534U); TEST_CHECK_TRUE(stage == 65530U);
  s1.step_back_window(stage = 65535U); TEST_CHECK_TRUE(stage == 65535U);
  s1.step_back_window(stage = 65536U); TEST_CHECK_TRUE(stage == 65535U);
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
