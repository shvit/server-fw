/*
 * tftpSmBuf_test.cpp
 *
 *  Created on: 13 апр. 2021 г.
 *      Author: svv
 */



#include "test.h"
#include "../tftpSmBuf.h"

UNIT_TEST_SUITE_BEGIN(tftp_base)

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(sm_buf_raw, "Check raw()")

  tftp::SmBuf b(16,0);

  // write buffer = 0x00,0x01,...,0x0f
  b.raw<int16_t>(0) = 0x0100;
  b.raw<int16_t>(2) = 0x0302;
  b.raw<int32_t>(4) = 0x07060504;
  b.raw<int64_t>(8) = 0x0f0e0d0c0b0a0908;

  const tftp::SmBuf & cb = b;

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

UNIT_TEST_CASE_BEGIN(sm_buf_get_ntoh, "Check get_ntoh()")

  tftp::SmBuf b(16,0);

  b.raw<int64_t>(0) = 0x0f0e0d0c0b0a0908;

  const tftp::SmBuf & cb = b;

  TEST_CHECK_TRUE(b.get_ntoh< int16_t>(0) == ( int16_t)0x0809);
  TEST_CHECK_TRUE(b.get_ntoh< int16_t>(1) == ( int16_t)0x090a);
  TEST_CHECK_TRUE(b.get_ntoh< int16_t>(2) == ( int16_t)0x0a0b);
  TEST_CHECK_TRUE(b.get_ntoh< int16_t>(3) == ( int16_t)0x0b0c);

  TEST_CHECK_TRUE(cb.get_ntoh< int32_t>(4) == ( int32_t)0x0c0d0e0f);
  TEST_CHECK_TRUE(cb.get_ntoh< int64_t>(0) == ( int64_t)0x08090a0b0c0d0e0f);

  b.set_hton(0, (int16_t)0x1234);
  TEST_CHECK_TRUE(b.get_ntoh< int16_t>(0) == ( int16_t)0x1234);
  b.set_hton(2, (int16_t)0x5678);
  TEST_CHECK_TRUE(b.get_ntoh< int32_t>(0) == ( int32_t)0x12345678);

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
