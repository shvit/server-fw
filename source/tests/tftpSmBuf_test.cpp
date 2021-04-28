/**
 * \file tftpSmBuf_test.cpp
 * \brief Unit-tests for class SmBuf
 *
 *  License GPL-3.0
 *
 *  \date   13-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include "test.h"
#include "../tftpSmBuf.h"

UNIT_TEST_SUITE_BEGIN(tftp_base)

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sm_buf_valid, "Check is_valid()")

  tftp::SmBuf b1;
  const tftp::SmBuf & cb1 = b1;

  TEST_CHECK_FALSE(cb1.is_valid(0U, 0U));
  TEST_CHECK_FALSE(cb1.is_valid(0U, 1U));
  TEST_CHECK_FALSE(cb1.is_valid(1U, 0U));
  TEST_CHECK_FALSE(cb1.is_valid(1U, 1U));

  tftp::SmBuf b2(16,0);
  const tftp::SmBuf & cb2 = b2;

  TEST_CHECK_TRUE (cb2.is_valid( 0U,  0U));
  TEST_CHECK_TRUE (cb2.is_valid( 0U, 16U));
  TEST_CHECK_TRUE (cb2.is_valid(15U,  1U));
  TEST_CHECK_TRUE (cb2.is_valid( 1U, 15U));
  TEST_CHECK_FALSE(cb2.is_valid( 0U, 17U));
  TEST_CHECK_FALSE(cb2.is_valid(16U,  0U));


//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sm_buf_raw, "Check raw()")

  tftp::SmBuf b(16,0);
  const tftp::SmBuf & cb = b;

  // write buffer = 0x00,0x01,...,0x0f
  b.raw<int16_t>(0) = 0x0100;
  b.raw<int16_t>(2) = 0x0302;
  b.raw<int32_t>(4) = 0x07060504;
  b.raw<int64_t>(8) = 0x0f0e0d0c0b0a0908;

  // Check types
  TEST_CHECK_TRUE((std::is_same_v<decltype( b.raw <int8_t >(0)), int8_t & >));
  TEST_CHECK_TRUE((std::is_same_v<decltype( b.raw <int16_t>(0)), int16_t &>));
  TEST_CHECK_TRUE((std::is_same_v<decltype( b.raw <int32_t>(0)), int32_t &>));
  TEST_CHECK_TRUE((std::is_same_v<decltype( b.raw <int64_t>(0)), int64_t &>));
  TEST_CHECK_TRUE((std::is_same_v<decltype(cb.raw <int8_t >(0)), const int8_t &>));
  TEST_CHECK_TRUE((std::is_same_v<decltype(cb.raw <int16_t>(0)), const int16_t &>));
  TEST_CHECK_TRUE((std::is_same_v<decltype(cb.raw <int32_t>(0)), const int32_t &>));
  TEST_CHECK_TRUE((std::is_same_v<decltype(cb.raw <int64_t>(0)), const int64_t &>));

  // read 1 bytes raw
  TEST_CHECK_TRUE(b.raw < int8_t >(0) == (int8_t)0x00);
  TEST_CHECK_TRUE(b.raw < int8_t >(1) == (int8_t)0x01);
  TEST_CHECK_TRUE(b.raw <uint8_t >(0) == (uint8_t)0x00U);
  TEST_CHECK_TRUE(b.raw <uint8_t >(1) == (uint8_t)0x01U);

  // read 2 bytes raw
  TEST_CHECK_TRUE(b.raw < int16_t>(0) == ( int16_t)0x0100);
  TEST_CHECK_TRUE(b.raw < int16_t>(1) == ( int16_t)0x0201);
  TEST_CHECK_TRUE(b.raw < int16_t>(2) == ( int16_t)0x0302);
  TEST_CHECK_TRUE(b.raw < int16_t>(3) == ( int16_t)0x0403);
  TEST_CHECK_TRUE(b.raw < int16_t>(4) == ( int16_t)0x0504);
  TEST_CHECK_TRUE(b.raw <uint16_t>(0) == (uint16_t)0x0100U);
  TEST_CHECK_TRUE(b.raw <uint16_t>(1) == (uint16_t)0x0201U);
  TEST_CHECK_TRUE(b.raw <uint16_t>(2) == (uint16_t)0x0302U);
  TEST_CHECK_TRUE(b.raw <uint16_t>(3) == (uint16_t)0x0403U);
  TEST_CHECK_TRUE(b.raw <uint16_t>(4) == (uint16_t)0x0504U);

  // read 4 bytes raw
  TEST_CHECK_TRUE(b.raw < int32_t >( 0) == ( int32_t)0x03020100);
  TEST_CHECK_TRUE(b.raw <uint32_t >( 4) == (uint32_t)0x07060504U);
  TEST_CHECK_TRUE(b.raw < int32_t >(12) == ( int32_t)0x0f0e0d0c);

  // read 8 bytes raw
  TEST_CHECK_TRUE(cb.raw < int64_t >( 0) == ( int64_t)0x0706050403020100);
  TEST_CHECK_TRUE(cb.raw <uint64_t >( 0) == (uint64_t)0x0706050403020100U);
  TEST_CHECK_TRUE(cb.raw < int64_t >( 7) == ( int64_t)0x0e0d0c0b0a090807);

  // check exception
  CHK_EXCEPTION__INV_ARG(b.raw <int8_t >(16));
  CHK_EXCEPTION__INV_ARG(cb.raw <int16_t>(15));

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sm_buf_get_set, "Check methods: set/get ntoh/raw")

  tftp::SmBuf b(16,0);
  const tftp::SmBuf & cb = b;

  b.raw<int64_t>(0) = 0x0f0e0d0c0b0a0908;

  // ntoh
  TEST_CHECK_TRUE( b.get_ntoh< int16_t>(0) == ( int16_t)0x0809);
  TEST_CHECK_TRUE( b.get_ntoh< int16_t>(1) == ( int16_t)0x090a);
  TEST_CHECK_TRUE(cb.get_ntoh< int16_t>(2) == ( int16_t)0x0a0b);
  TEST_CHECK_TRUE(cb.get_ntoh< int16_t>(3) == ( int16_t)0x0b0c);

  TEST_CHECK_TRUE(cb.get_ntoh< int32_t>(4) == ( int32_t)0x0c0d0e0f);
  TEST_CHECK_TRUE(cb.get_ntoh< int64_t>(0) == ( int64_t)0x08090a0b0c0d0e0f);

  b.set_hton(0, (int16_t)0x1234);
  TEST_CHECK_TRUE(b.get_ntoh< int16_t>(0U) == ( int16_t)0x1234);
  TEST_CHECK_TRUE(b.get_ntoh< int16_t>(2U) == ( int16_t)0x0a0b);
  b.set_hton(2, (int16_t)0x5678);
  TEST_CHECK_TRUE(b.get_ntoh< int32_t>(0) == ( int32_t)0x12345678);
  b.set_hton(4, (int32_t)0x90abcdef);
  TEST_CHECK_TRUE(cb.get_ntoh< int64_t>(0) == ( int64_t)0x1234567890abcdef);

  // raw
  TEST_CHECK_TRUE(b.set_raw(0, (int64_t) 0x0f0e0d0c0b0a0908) == (ssize_t)sizeof(int64_t));

  TEST_CHECK_TRUE(cb.get_raw< int16_t>(0) == ( int16_t)0x0908);
  TEST_CHECK_TRUE(cb.get_raw< int16_t>(1) == ( int16_t)0x0a09);
  TEST_CHECK_TRUE(cb.get_raw< int16_t>(2) == ( int16_t)0x0b0a);
  TEST_CHECK_TRUE(cb.get_raw< int16_t>(3) == ( int16_t)0x0c0b);

  // check exception
  CHK_EXCEPTION__INV_ARG(cb.get_ntoh <int8_t >(16));
  CHK_EXCEPTION__INV_ARG(cb.get_raw <int16_t>(15));
  CHK_EXCEPTION__INV_ARG( b.set_hton(16, (int8_t)1));
  CHK_EXCEPTION__INV_ARG( b.set_raw(15, (int16_t)1));

  //
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sm_buf_str, "Check methods: set/get/eqv string")

  tftp::SmBuf b(32,0);
  const tftp::SmBuf & cb = b;

  TEST_CHECK_TRUE (cb.get_string(0) == "");
  TEST_CHECK_TRUE (cb.eqv_string(0, ""));
  TEST_CHECK_TRUE (cb.eqv_string(0, "", true));
  TEST_CHECK_FALSE(cb.eqv_string(0, "1"));
  TEST_CHECK_FALSE(cb.eqv_string(0, "1", true));

  TEST_CHECK_TRUE ( b.set_string(2, "12345678") == 8);

  TEST_CHECK_TRUE (cb.get_string(0) == "");
  TEST_CHECK_TRUE (cb.eqv_string(0, ""));
  TEST_CHECK_TRUE (cb.eqv_string(0, "", true));
  TEST_CHECK_TRUE (cb.get_string(2) == "12345678");
  TEST_CHECK_TRUE (cb.eqv_string(2, "12345678"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "12345678", true));
  TEST_CHECK_TRUE (cb.eqv_string(2, "1"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "12"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "123"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "1234"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "12345"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "123456"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "1234567"));
  TEST_CHECK_TRUE (cb.eqv_string(2, "12345678"));
  TEST_CHECK_FALSE(cb.eqv_string(2, "1234567", true));
  TEST_CHECK_TRUE (cb.eqv_string(2, "12345678", true));
  TEST_CHECK_TRUE (cb.get_string(5) == "45678");

  TEST_CHECK_TRUE(b.set_string(3, "abc", true) == 4);
  TEST_CHECK_TRUE(cb.get_string(2) == "1abc");
  TEST_CHECK_TRUE(cb.get_string(6) == "");
  TEST_CHECK_TRUE(cb.get_string(7) == "678");

  TEST_CHECK_TRUE (cb.eqv_string(7, "6"));
  TEST_CHECK_FALSE(cb.eqv_string(7, "6", true));
  TEST_CHECK_TRUE (cb.eqv_string(7, "67"));
  TEST_CHECK_FALSE(cb.eqv_string(7, "67", true));
  TEST_CHECK_TRUE (cb.eqv_string(7, "678"));
  TEST_CHECK_TRUE (cb.eqv_string(7, "678", true));
  TEST_CHECK_FALSE(cb.eqv_string(8, "678"));
  TEST_CHECK_FALSE(cb.eqv_string(8, "678", true));
  TEST_CHECK_FALSE(cb.eqv_string(30, "678"));
  TEST_CHECK_FALSE(cb.eqv_string(30, "678", true));
  TEST_CHECK_FALSE(cb.eqv_string(90, "678"));
  TEST_CHECK_FALSE(cb.eqv_string(90, "678", true));

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
