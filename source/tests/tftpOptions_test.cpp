/*
 * tftpOptions_test.cpp
 *
 *  Created on: 16 апр. 2021 г.
 *      Author: svv
 */

#include "../tftpCommon.h"
#include "../tftpOptions.h"
#include "test.h"

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(Options)

//------------------------------------------------------------------------------

class Options_test : public tftp::Options
{
public:
  using Options::request_type_;
  using Options::filename_;
  using Options::transfer_mode_;
  using Options::blksize_;
  using Options::timeout_;
  using Options::tsize_;
  using Options::windowsize_;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(opt_base, "Check base operations")

  // default construct
  Options_test o;

  TEST_CHECK_TRUE(o.request_type_ == tftp::SrvReq::unknown);
  TEST_CHECK_TRUE(o.transfer_mode_ == tftp::TransfMode::unknown);
  TEST_CHECK_TRUE(o.filename_.size() == 0U);


  TEST_CHECK_FALSE(std::get<0>(o.blksize_));
  TEST_CHECK_TRUE (std::get<1>(o.blksize_) == tftp::constants::dflt_blksize);
  TEST_CHECK_FALSE(std::get<0>(o.timeout_));
  TEST_CHECK_TRUE (std::get<1>(o.timeout_) == tftp::constants::dflt_timeout);
  TEST_CHECK_FALSE(std::get<0>(o.tsize_));
  TEST_CHECK_TRUE (std::get<1>(o.tsize_) == tftp::constants::dflt_tsize);
  TEST_CHECK_FALSE(std::get<0>(o.windowsize_));
  TEST_CHECK_TRUE (std::get<1>(o.windowsize_) == tftp::constants::dflt_windowsize);

  TEST_CHECK_TRUE(o.request_type() == tftp::SrvReq::unknown);
  TEST_CHECK_TRUE(o.transfer_mode() == tftp::TransfMode::unknown);
  TEST_CHECK_TRUE(o.filename() == "");
  TEST_CHECK_TRUE(o.blksize() == tftp::constants::dflt_blksize);
  TEST_CHECK_TRUE(o.timeout() == tftp::constants::dflt_timeout);
  TEST_CHECK_TRUE(o.tsize() == tftp::constants::dflt_tsize);
  TEST_CHECK_TRUE(o.windowsize() == tftp::constants::dflt_windowsize);

  // Fill values

  Options_test & ro = o;
  const Options_test & roc = o;

  o.blksize_    = {true, 111111};
  o.timeout_    = {true, 222222};
  o.tsize_      = {true, 333333};
  o.windowsize_ = {true, 444444};
  o.request_type_ = tftp::SrvReq::read;
  o.transfer_mode_ = tftp::TransfMode::mail;
  o.filename_ = "name.file";

  // Copy/move constructors

  Options_test o11(o);
  Options_test o12(ro);
  Options_test o13(roc);
  Options_test o14(std::move(o));

  TEST_CHECK_TRUE(o11.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o11.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o11.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o11.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o11.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o11.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o11.windowsize_) == 444444);

  TEST_CHECK_TRUE(o12.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o12.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o12.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o12.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o12.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o12.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o12.windowsize_) == 444444);

  TEST_CHECK_TRUE(o13.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o13.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o13.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o13.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o13.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o13.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o13.windowsize_) == 444444);

  TEST_CHECK_TRUE(o14.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o14.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o14.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o14.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o14.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o14.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o14.windowsize_) == 444444);

  TEST_CHECK_TRUE(o.filename_ == "");

  // Copy/move operators
  o = std::move(o14);
  Options_test o21 = o11;
  Options_test o22 = o12;
  Options_test o23 = o13;

  TEST_CHECK_TRUE(o21.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o21.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o21.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o21.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o21.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o21.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o21.windowsize_) == 444444);

  TEST_CHECK_TRUE(o22.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o22.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o22.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o22.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o22.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o22.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o22.windowsize_) == 444444);

  TEST_CHECK_TRUE(o23.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o23.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o23.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o23.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o23.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o23.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o23.windowsize_) == 444444);

  TEST_CHECK_TRUE(o.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o.transfer_mode_ == tftp::TransfMode::mail);
  TEST_CHECK_TRUE(o.filename_ == "name.file");
  TEST_CHECK_TRUE(std::get<1>(o.blksize_) == 111111);
  TEST_CHECK_TRUE(std::get<1>(o.timeout_) == 222222);
  TEST_CHECK_TRUE(std::get<1>(o.tsize_) == 333333);
  TEST_CHECK_TRUE(std::get<1>(o.windowsize_) == 444444);

  TEST_CHECK_TRUE(o14.filename_ == "");

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(opt_buffer_parse, "Check buffer_parse()")

// Stage 1 - full empty
{
  tftp::SmBuf b_pkt;

  Options_test o;

  TEST_CHECK_FALSE(o.buffer_parse(b_pkt, b_pkt.size(), nullptr));

  TEST_CHECK_TRUE(o.request_type_ == tftp::SrvReq::unknown);
  TEST_CHECK_TRUE(o.filename_ == "");
  TEST_CHECK_TRUE(o.transfer_mode_ == tftp::TransfMode::unknown);

  TEST_CHECK_FALSE(std::get<0>(o.blksize_));
  TEST_CHECK_FALSE(std::get<0>(o.timeout_));
  TEST_CHECK_FALSE(std::get<0>(o.tsize_));
  TEST_CHECK_FALSE(std::get<0>(o.windowsize_));

  TEST_CHECK_TRUE(o.blksize() == tftp::constants::dflt_blksize);
  TEST_CHECK_TRUE(o.timeout() == tftp::constants::dflt_timeout);
  TEST_CHECK_TRUE(o.tsize()   == tftp::constants::dflt_tsize);
  TEST_CHECK_TRUE(o.windowsize() == tftp::constants::dflt_windowsize);

  TEST_CHECK_FALSE(o.was_set_blksize());
  TEST_CHECK_FALSE(o.was_set_timeout());
  TEST_CHECK_FALSE(o.was_set_tsize());
  TEST_CHECK_FALSE(o.was_set_windowsize());
}

// Stage 2 - filled zero buffer
{
  tftp::SmBuf b_pkt(500U,0);

  Options_test o;

  TEST_CHECK_FALSE(o.buffer_parse(b_pkt, b_pkt.size(), nullptr));

  TEST_CHECK_TRUE(o.request_type_ == tftp::SrvReq::unknown);
  TEST_CHECK_TRUE(o.filename_ == "");
  TEST_CHECK_TRUE(o.transfer_mode_ == tftp::TransfMode::unknown);

  TEST_CHECK_FALSE(std::get<0>(o.blksize_));
  TEST_CHECK_FALSE(std::get<0>(o.timeout_));
  TEST_CHECK_FALSE(std::get<0>(o.tsize_));
  TEST_CHECK_FALSE(std::get<0>(o.windowsize_));

  TEST_CHECK_TRUE(o.blksize()    == tftp::constants::dflt_blksize);
  TEST_CHECK_TRUE(o.timeout()    == tftp::constants::dflt_timeout);
  TEST_CHECK_TRUE(o.tsize()      == tftp::constants::dflt_tsize);
  TEST_CHECK_TRUE(o.windowsize() == tftp::constants::dflt_windowsize);

  TEST_CHECK_FALSE(o.was_set_blksize());
  TEST_CHECK_FALSE(o.was_set_timeout());
  TEST_CHECK_FALSE(o.was_set_tsize());
  TEST_CHECK_FALSE(o.was_set_windowsize());
}

// Stage 3 - fimple, w/o options
{
  tftp::SmBuf b_pkt
  {
    0,1,
    'f','i','l','e','n','a','m','e','.','t','x','t',0,
    'm','a','i','l',0
  };

  Options_test o;

  TEST_CHECK_TRUE(o.buffer_parse(b_pkt, b_pkt.size(), nullptr));

  TEST_CHECK_TRUE(o.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o.filename_ == "filename.txt");
  TEST_CHECK_TRUE(o.transfer_mode_ == tftp::TransfMode::mail);

  TEST_CHECK_FALSE(std::get<0>(o.blksize_));
  TEST_CHECK_FALSE(std::get<0>(o.timeout_));
  TEST_CHECK_FALSE(std::get<0>(o.tsize_));
  TEST_CHECK_FALSE(std::get<0>(o.windowsize_));

  TEST_CHECK_TRUE(o.blksize()    == tftp::constants::dflt_blksize);
  TEST_CHECK_TRUE(o.timeout()    == tftp::constants::dflt_timeout);
  TEST_CHECK_TRUE(o.tsize()      == tftp::constants::dflt_tsize);
  TEST_CHECK_TRUE(o.windowsize() == tftp::constants::dflt_windowsize);

  TEST_CHECK_FALSE(o.was_set_blksize());
  TEST_CHECK_FALSE(o.was_set_timeout());
  TEST_CHECK_FALSE(o.was_set_tsize());
  TEST_CHECK_FALSE(o.was_set_windowsize());
}


// Stage 4 - full data buffer, wrong options
{
  tftp::SmBuf b_pkt
  {
    0,1,
    'f','i','l','e','n','a','m','e','.','t','x','t',0,
    'm','a','i','l',0,
    'b','l','k','s','i','z','e',0,0,
    't','i','m','e','o','u','t',0,'-','9',0,
    't','s','i','z','e',0,'2','1','2','3','z',0,
    'w','i','n','d','o','w','s','i','z','e',0,'e','f',0
  };

  Options_test o;

  TEST_CHECK_TRUE(o.buffer_parse(b_pkt, b_pkt.size(), nullptr));

  TEST_CHECK_TRUE(o.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o.filename_ == "filename.txt");
  TEST_CHECK_TRUE(o.transfer_mode_ == tftp::TransfMode::mail);

  TEST_CHECK_FALSE(std::get<0>(o.blksize_));
  TEST_CHECK_FALSE(std::get<0>(o.timeout_));
  TEST_CHECK_FALSE(std::get<0>(o.tsize_));
  TEST_CHECK_FALSE(std::get<0>(o.windowsize_));

  TEST_CHECK_TRUE(o.blksize()    == tftp::constants::dflt_blksize);
  TEST_CHECK_TRUE(o.timeout()    == tftp::constants::dflt_timeout);
  TEST_CHECK_TRUE(o.tsize()      == tftp::constants::dflt_tsize);
  TEST_CHECK_TRUE(o.windowsize() == tftp::constants::dflt_windowsize);

  TEST_CHECK_FALSE(o.was_set_blksize());
  TEST_CHECK_FALSE(o.was_set_timeout());
  TEST_CHECK_FALSE(o.was_set_tsize());
  TEST_CHECK_FALSE(o.was_set_windowsize());
}

// Stage 4 - full data buffer, good options
{
  tftp::SmBuf b_pkt
  {
    0,1,
    'f','i','l','e','n','a','m','e','.','t','x','t',0,
    'm','a','i','l',0,
    'b','l','k','s','i','z','e',0,'1','0','2','4',0,
    't','i','m','e','o','u','t',0,'1','0',0,
    't','s','i','z','e',0,'2','0','0','0','1','2','3',0,
    'w','i','n','d','o','w','s','i','z','e',0,'2','0',0
  };

  Options_test o;

  TEST_CHECK_TRUE(o.buffer_parse(b_pkt, b_pkt.size(), nullptr));

  TEST_CHECK_TRUE(o.request_type_ == tftp::SrvReq::read);
  TEST_CHECK_TRUE(o.filename_ == "filename.txt");
  TEST_CHECK_TRUE(o.transfer_mode_ == tftp::TransfMode::mail);

  TEST_CHECK_TRUE(std::get<0>(o.blksize_));
  TEST_CHECK_TRUE(std::get<0>(o.timeout_));
  TEST_CHECK_TRUE(std::get<0>(o.tsize_));
  TEST_CHECK_TRUE(std::get<0>(o.windowsize_));

  TEST_CHECK_TRUE(o.blksize() == 1024);
  TEST_CHECK_TRUE(o.timeout() == 10);
  TEST_CHECK_TRUE(o.tsize()   == 2000123);
  TEST_CHECK_TRUE(o.windowsize() == 20);

  TEST_CHECK_TRUE(o.was_set_blksize());
  TEST_CHECK_TRUE(o.was_set_timeout());
  TEST_CHECK_TRUE(o.was_set_tsize());
  TEST_CHECK_TRUE(o.was_set_windowsize());
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
