/**
 * \file tftpSmBufEx_test.cpp
 * \brief Unit-tests for class SmBufEx
 *
 *  License GPL-3.0
 *
 *  \date   18-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include "test.h"
#include "../tftpSmBufEx.h"

UNIT_TEST_SUITE_BEGIN(SmBufEx)

class SmBufEx_test: public tftp::SmBufEx
{
public:
  using SmBufEx::data_size_;
  using SmBufEx::val_int_bigendian_;
  using SmBufEx::val_str_zeroend_;
  using SmBufEx::SmBufEx;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(constructors, "Check constructors")

START_ITER("Check can construnct")
{
  TEST_CHECK_FALSE((std::is_constructible_v<tftp::SmBufEx, void>));
  TEST_CHECK_TRUE ((std::is_constructible_v<tftp::SmBufEx, size_t>));
  TEST_CHECK_TRUE ((std::is_constructible_v<tftp::SmBufEx, int>));
  TEST_CHECK_TRUE ((std::is_constructible_v<tftp::SmBufEx, size_t, std::string>));
  TEST_CHECK_TRUE ((std::is_constructible_v<tftp::SmBufEx, size_t, uint16_t, std::string>));
}

START_ITER("Construnctor without data")
{
  tftp::SmBufEx b{1024U};
  TEST_CHECK_TRUE (b.size() == 1024U);
  TEST_CHECK_TRUE (b.data_size() == 0U);
  TEST_CHECK_TRUE (b.is_bigendian());
  TEST_CHECK_FALSE(b.is_littleendian());
  TEST_CHECK_TRUE (b.is_zeroend());
}

START_ITER("Construnctor with data (1 chars)")
{
  tftp::SmBufEx b(20U, 'a','z','c');
  TEST_CHECK_TRUE(b.size() == 20U);
  TEST_CHECK_TRUE(b.data_size() == 3U);
  TEST_CHECK_TRUE(b[0U] == 'a');
  TEST_CHECK_TRUE(b[1U] == 'z');
  TEST_CHECK_TRUE(b[2U] == 'c');
  TEST_CHECK_TRUE (b.is_bigendian());
  TEST_CHECK_FALSE(b.is_littleendian());
  TEST_CHECK_TRUE (b.is_zeroend());
  b.clear();
  TEST_CHECK_TRUE(b.size() == 20U);
  TEST_CHECK_TRUE(b.data_size() == 0U);
}

START_ITER("Construnctor with data (2) string")
{
  tftp::SmBufEx b(20U, (uint16_t) 5, std::string{"testing"});
  TEST_CHECK_TRUE(b.size() == 20U);
  TEST_CHECK_TRUE(b.data_size() == 10U);
  TEST_CHECK_TRUE(b[2U] == 't');
  TEST_CHECK_TRUE(b[8U] == 'g');
  TEST_CHECK_TRUE(b[9U] == '\0');
  TEST_CHECK_TRUE(b.is_zeroend());
  b.set_not_zeroend();
  TEST_CHECK_FALSE(b.is_zeroend());
  b.push_data("qwe");
  TEST_CHECK_TRUE(b.size() == 20U);
  TEST_CHECK_TRUE(b.data_size() == 13U);
}

START_ITER("Construnctor with data (3) big endian")
{
  tftp::SmBufEx b(30U, (int32_t) 0x01020304);
  TEST_CHECK_TRUE(b.size() == 30U);
  //TEST_CHECK_TRUE(b.data_size() == 0U);
  TEST_CHECK_TRUE (b.is_bigendian());
  TEST_CHECK_FALSE(b.is_littleendian());
  //b.push_data((int32_t) 0x01020304);
  //TEST_CHECK_TRUE(b.size() == 30U);
  TEST_CHECK_TRUE(b.data_size() == 4U);
  TEST_CHECK_TRUE(b[0U] == '\1');
  TEST_CHECK_TRUE(b[1U] == '\2');
  TEST_CHECK_TRUE(b[2U] == '\3');
  TEST_CHECK_TRUE(b[3U] == '\4');

  b.set_littleendian();
  TEST_CHECK_FALSE(b.is_bigendian());
  TEST_CHECK_TRUE (b.is_littleendian());
  b.push_data((int32_t) 0x11223344);
  TEST_CHECK_TRUE(b.size() == 30U);
  TEST_CHECK_TRUE(b.data_size() == 8U);
  TEST_CHECK_TRUE(b[4U] == 0x44);
  TEST_CHECK_TRUE(b[5U] == 0x33);
  TEST_CHECK_TRUE(b[6U] == 0x22);
  TEST_CHECK_TRUE(b[7U] == 0x11);
}

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
