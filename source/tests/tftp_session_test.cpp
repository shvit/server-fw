#include <iostream>
#include <netinet/in.h>

#include "../tftp_common.h"
#include "../tftp_session.h"
#include "test.h"     

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(tftp_session)

class test_session: public tftp::session
{
public:
  // get protected property
  auto & get_request_type()  { return request_type_; };
  auto & get_transfer_mode() { return transfer_mode_; };
  auto & get_client()        { return client_; };
  auto & get_filename()      { return filename_; };
  auto   get_opt_blksize()   { return opt_blksize_; };
  auto   get_opt_timeout()   { return opt_timeout_; };
  auto   get_opt_tsize()     { return opt_tsize_; };

  // forward protected methods
  auto was_error() { return tftp::session::was_error(); };

  template<typename ... Ts>
  auto set_error_if_first(Ts && ... args)
  {
    return tftp::session::set_error_if_first(std::forward<Ts>(args) ...);
  }

};

[[maybe_unused]]
auto logger = [](const int level, std::string_view message)
  {
      std::cout << "{" << tftp::to_string_lvl(level) << "} " << message << std::endl; // << std::flush();
  };

//=================================================================================================================================



UNIT_TEST_CASE_BEGIN(session, "check session")

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

    std::vector<char> pkt1{0,2,
                           'f','i','l','e','n','a','m','e','.','t','x','t',0,
                           'o','c','t','e','t',0,
                           'b','l','k','s','i','z','e',0,'1','0','2','4',0,
                           't','i','m','e','o','u','t',0,'1','0',0,
                           't','s','i','z','e',0,'2','0','0','0','1','2','3',0};

    test_session s1;
    //s1.set_logger(logger);
    s1.set_root_dir(tmp_dir);
    s1.set_local_base("0.0.0.0:7777");
    TEST_CHECK_TRUE(s1.init(static_cast<tftp::Buf::const_iterator>((char *) & a),
                            static_cast<tftp::Buf::const_iterator>((char *) & a) + sizeof(a),
                            pkt1.cbegin(),
                            pkt1.cend()));
    TEST_CHECK_TRUE(s1.get_request_type() == tftp::SrvReq::write);
    TEST_CHECK_TRUE(s1.get_transfer_mode() == tftp::TransfMode::octet);
    TEST_CHECK_TRUE(s1.get_client().size()==sizeof(a) &&
                    std::equal(s1.get_client().cbegin(),
                               s1.get_client().cend(),
                               (char *) & a));
    TEST_CHECK_TRUE(std::get<0>(s1.get_opt_blksize()));
    TEST_CHECK_TRUE(std::get<1>(s1.get_opt_blksize()) == 1024U);
    TEST_CHECK_TRUE(std::get<0>(s1.get_opt_timeout()));
    TEST_CHECK_TRUE(std::get<1>(s1.get_opt_timeout()) == 10);
    TEST_CHECK_TRUE(std::get<0>(s1.get_opt_tsize()));
    TEST_CHECK_TRUE(std::get<1>(s1.get_opt_tsize()) == 2000123);
    TEST_CHECK_TRUE(s1.get_filename() == "filename.txt");

    TEST_CHECK_FALSE(s1.was_error());
    s1.set_error_if_first(909U, "Test error");
    TEST_CHECK_TRUE(s1.was_error());
  }
UNIT_TEST_CASE_END

//---------------------------------------------------------------------------------------------------------------------------------


UNIT_TEST_SUITE_END
