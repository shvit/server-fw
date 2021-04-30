/*
 * tftpBase_test.cpp
 *
 *  Created on: 13 апр. 2021 г.
 *      Author: svv
 */

#include <netinet/in.h>

#include "../tftpCommon.h"
#include "../tftpBase.h"
#include "test.h"

UNIT_TEST_SUITE_BEGIN(tftp_base)


class tst_Base: public tftp::Base
{
public:
  using tftp::Base::settings_;
};

//------------------------------------------------------------------------------
UNIT_TEST_CASE_BEGIN(main, "")

  tst_Base b;

  //TEST_CHECK_TRUE(b.get_buf_item_raw <uint16_t>(a1, 0) == 0x0100U);

//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
