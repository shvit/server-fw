#include <netinet/in.h>

#include "../tftpCommon.h"
#include "../tftpSession.h"
#include "test.h"     

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(tftp_session)

class test_session: public tftp::Session
{
public:

  //using tftp::Session::request_type_;
  //using tftp::Session::transfer_mode_;
  //using tftp::Session::client_;
  //using tftp::Session::filename_;
  //using tftp::Session::opt_blksize_;
  //using tftp::Session::opt_timeout_;
  //using tftp::Session::opt_tsize_;
  using tftp::Session::opt_;
  using tftp::Session::cl_addr_;
  //using tftp::Session::opt;

  using tftp::Session::was_error;
  using tftp::Session::set_error_if_first;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(session, "check session (OLD)")
/*
  // 0 prepare
  START_ITER("prepare - cretae temporary directory");
  {
    TEST_CHECK_TRUE(temp_directory_create());
  }

  START_ITER("init");
  {
    struct sockaddr_in a;
    a.sin_port=8888U;
    a.sin_addr.s_addr=0x01020304U;

    tftp::Buf pkt1
    {
      0,2,
      'f','i','l','e','n','a','m','e','.','t','x','t',0,
      'o','c','t','e','t',0,
      'b','l','k','s','i','z','e',0,'1','0','2','4',0,
      't','i','m','e','o','u','t',0,'1','0',0,
      't','s','i','z','e',0,'2','0','0','0','1','2','3',0
    };

    test_session s1;
    s1.set_root_dir(tmp_dir);
    s1.set_local_base("0.0.0.0:7777");
    TEST_CHECK_TRUE(s1.init(
        static_cast<tftp::Buf::const_iterator>((char *) & a),
        static_cast<tftp::Buf::const_iterator>((char *) & a) + sizeof(a),
        pkt1.cbegin(),
        pkt1.cend()));
    //TEST_CHECK_TRUE(s1.request_type_ == tftp::SrvReq::write);
    TEST_CHECK_TRUE(s1.transfer_mode_ == tftp::TransfMode::octet);
    TEST_CHECK_TRUE(s1.client_.size()==sizeof(a) &&
                    std::equal(s1.client_.cbegin(),
                               s1.client_.cend(),
                               (char *) & a));
    //TEST_CHECK_TRUE(std::get<0>(s1.opt_blksize_));
    //TEST_CHECK_TRUE(std::get<1>(s1.opt_blksize_) == 1024U);
    //TEST_CHECK_TRUE(std::get<0>(s1.opt_timeout_));
    //TEST_CHECK_TRUE(std::get<1>(s1.opt_timeout_) == 10);
    //TEST_CHECK_TRUE(std::get<0>(s1.opt_tsize_));
    //TEST_CHECK_TRUE(std::get<1>(s1.opt_tsize_) == 2000123);
    TEST_CHECK_TRUE(s1.filename_ == "filename.txt");

    TEST_CHECK_FALSE(s1.was_error());
    s1.set_error_if_first(909U, "Test error");
    TEST_CHECK_TRUE(s1.was_error());
  }

  */
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sess_init, "check prepare()")

  tftp::Addr b_addr;

  b_addr.set_family(AF_INET);
  b_addr.set_port(0x0506);
  //b_addr.set_hton(4U, (int32_t) 0x01020304);
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
