/**
 * \file tftpDataMgr_test.cpp
 * \brief Unit-tests for class DataMgr
 *
 *  License GPL-3.0
 *
 *  \date 13-sep-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2.1
 */

#include "test.h"
#include "../tftpCommon.h"
#include "../tftpDataMgr.h"
#include "../tftpOptions.h"

UNIT_TEST_SUITE_BEGIN(DataMgr)

using namespace unit_tests;

//------------------------------------------------------------------------------

/** \brief Helper class for access to DataMgr protected fields
 */
class DataMgr_test: public tftp::DataMgr
{
public:

  DataMgr_test(): tftp::DataMgr() {};

  virtual ~DataMgr_test() {};

  virtual bool active() const override { return true; };

  virtual bool init(
      tftp::pSettings & sett,
      tftp::fSetError cb_error,
      const tftp::Options & opt) override { set_error_ = cb_error; return true; };

  virtual auto write(
      tftp::SmBufEx::const_iterator buf_begin,
      tftp::SmBufEx::const_iterator buf_end,
      const size_t & position) -> ssize_t override { return 0; };

  virtual auto read(
      tftp::SmBufEx::iterator buf_begin,
      tftp::SmBufEx::iterator buf_end,
      const size_t & position) -> ssize_t override { return 0; };

  virtual void close() override {};

  using tftp::DataMgr::set_error_;
  using tftp::DataMgr::match_md5;
  using tftp::DataMgr::set_error_if_first;


};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(main, "Main checks")

START_ITER("check match_md5()")
{
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5("server-fw"));
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5("server-fw.md5"));
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4z"));
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4z.md5"));
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5("2fdf093688bb7cef7c05b 1ffcc71ff4e"));
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4e.md5"));
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5("172775dbdee46e00a422235475244db6.md5"));
  TEST_CHECK_FALSE(DataMgr_test{}.match_md5(""));

  TEST_CHECK_TRUE (DataMgr_test{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4e"));
  TEST_CHECK_TRUE (DataMgr_test{}.match_md5("172775dbdee46e00a422235475244db6"));
  TEST_CHECK_TRUE (DataMgr_test{}.match_md5("00000000000000000000000000000000"));
  TEST_CHECK_TRUE (DataMgr_test{}.match_md5("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));
}

START_ITER("check set_error_if_first()")
{
  DataMgr_test dm{};

  TEST_CHECK_TRUE(dm.set_error_ == nullptr);

  size_t ret=0U;
  auto cb = [&](const uint16_t a, std::string_view b)
    {
      ++ret;
    };

  tftp::pSettings p{nullptr};
  tftp::Options o{};

  dm.init(p, cb, o);
  dm.set_error_if_first(1,"error 1");
  dm.set_error_if_first(5,"error 5");
  TEST_CHECK_TRUE(ret == 2U);
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
