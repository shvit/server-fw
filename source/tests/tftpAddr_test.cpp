/**
 * \file tftpAddr_test.cpp
 * \brief TFTP address buffer class unit-tests
 *
 *  License GPL-3.0
 *
 *  \date   22-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <arpa/inet.h>

#include "../tftpAddr.h"
#include "test.h"

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(tftp_addr)

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(main, "Check main methods")

  tftp::Addr a;
  const tftp::Addr & ca = a;

  TEST_CHECK_TRUE( a.size() == tftp::constants::max_sockaddr_size);
  TEST_CHECK_TRUE(ca.size() == tftp::constants::max_sockaddr_size);
  TEST_CHECK_TRUE( a.data_size() == 0U);
  TEST_CHECK_TRUE(ca.data_size() == 0U);

  TEST_CHECK_TRUE( a.family() == 0U);
  TEST_CHECK_TRUE(ca.family() == 0U);

  TEST_CHECK_TRUE( a.port() == 0U);
  TEST_CHECK_TRUE(ca.port() == 0U);

  TEST_CHECK_TRUE( a.as_in().sin_family == 0U);
  TEST_CHECK_TRUE(ca.data_size() == sizeof(struct sockaddr_in));
  TEST_CHECK_TRUE( a.as_in().sin_port == 0U);
  TEST_CHECK_TRUE(ca.as_in().sin_family == 0U);
  TEST_CHECK_TRUE(ca.as_in().sin_port == 0U);

  TEST_CHECK_TRUE( a.as_in6().sin6_family == 0U);
  TEST_CHECK_TRUE(ca.data_size() == sizeof(struct sockaddr_in6));
  TEST_CHECK_TRUE( a.as_in6().sin6_port == 0U);
  TEST_CHECK_TRUE(ca.as_in6().sin6_family == 0U);
  TEST_CHECK_TRUE(ca.as_in6().sin6_port == 0U);

  TEST_CHECK_TRUE( a.str() == tftp::constants::unknown_addr);
  TEST_CHECK_TRUE(ca.str() == tftp::constants::unknown_addr);

  // as_in()
  a.set_family(AF_INET);
  TEST_CHECK_TRUE(ca.data_size() == sizeof(struct sockaddr_in));
  TEST_CHECK_TRUE(ca.str() == "0.0.0.0:0");
  a.set_port(0x3412U);
  a.as_in().sin_addr.s_addr = 0xe1e2e3e4U;
  TEST_CHECK_TRUE(ca.data_size() == sizeof(struct sockaddr_in));
  TEST_CHECK_TRUE(ca.family() == AF_INET);
  TEST_CHECK_TRUE(ca.port() == 0x3412U);
  TEST_CHECK_TRUE(ca.str() == "228.227.226.225:13330");

  // clear()
  a.clear();
  TEST_CHECK_TRUE(ca.as_in().sin_family == 0U);
  TEST_CHECK_TRUE(ca.as_in().sin_port == 0U);
  TEST_CHECK_TRUE(ca.as_in().sin_addr.s_addr == 0U);
  TEST_CHECK_TRUE(ca.data_size() == 0U);

  // as_in6()
  a.set_family(AF_INET6);
  a.set_port(0x3412);
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[0] = 0x01U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[1] = 0x02U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[2] = 0x03U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[3] = 0x04U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[4] = 0x05U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[5] = 0x06U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[6] = 0x07U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[7] = 0x08U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[8] = 0x09U;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[9] = 0x0aU;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[10] = 0x0bU;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[11] = 0x0cU;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[12] = 0x0dU;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[13] = 0x0eU;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[14] = 0x0fU;
  a.as_in6().sin6_addr.__in6_u.__u6_addr8[15] = 0xffU;
  TEST_CHECK_TRUE(ca.data_size() == sizeof(struct sockaddr_in6));
  TEST_CHECK_TRUE(ca.family() == AF_INET6);
  TEST_CHECK_TRUE(ca.port() == 0x3412U);
  TEST_CHECK_TRUE(ca.str() == "[102:304:506:708:90a:b0c:d0e:fff]:13330");

  // assign()
  struct sockaddr_in a4{0};
  a4.sin_family = AF_INET;
  a4.sin_port = 0x2301U;
  a4.sin_addr.s_addr = 0x04030201U;
  a.assign((const char *) & a4, sizeof(a4));
  TEST_CHECK_TRUE(ca.str() == "1.2.3.4:291");

  // as_sockaddr_ptr()
  TEST_CHECK_TRUE(a.as_sockaddr_ptr()->sa_family == AF_INET);
  TEST_CHECK_TRUE(a.as_sockaddr_ptr()->sa_data[0] == 0x01);
  TEST_CHECK_TRUE(a.as_sockaddr_ptr()->sa_data[1] == 0x23);
  TEST_CHECK_TRUE(a.as_sockaddr_ptr()->sa_data[2] == 0x01);
  TEST_CHECK_TRUE(a.as_sockaddr_ptr()->sa_data[3] == 0x02);
  TEST_CHECK_TRUE(a.as_sockaddr_ptr()->sa_data[4] == 0x03);
  TEST_CHECK_TRUE(a.as_sockaddr_ptr()->sa_data[5] == 0x04);

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
