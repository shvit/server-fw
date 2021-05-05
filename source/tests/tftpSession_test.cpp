#include <netinet/in.h>

#include "../tftpCommon.h"
#include "../tftpSession.h"
#include "test.h"     

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(Session)

class test_session: public tftp::Session
{
public:
  using tftp::Session::opt_;
  using tftp::Session::cl_addr_;
  using tftp::Session::was_error;
  using tftp::Session::set_error_if_first;
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

  test_session s1;

  TEST_CHECK_FALSE(s1.prepare(b_addr, b_pkt, b_pkt.size()));
  TEST_CHECK_TRUE(s1.opt_.request_type() == tftp::SrvReq::unknown);

  b_pkt.set_hton(0U, (int16_t)tftp::SrvReq::write);
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

UNIT_TEST_SUITE_END
