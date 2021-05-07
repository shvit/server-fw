/**
 * \file tftpCommon_test.cpp
 * \brief Unit-tests for common module
 *
 *  License GPL-3.0
 *
 *  \date   07-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include "../tftpCommon.h"
#include "test.h"     

UNIT_TEST_SUITE_BEGIN(Common)

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(to_string, "to_string()")

#define CHK_TO_STRING(T,NAME) \
    {\
      constexpr auto s1 = tftp::to_string(tftp::T::NAME);\
      TEST_CHECK_TRUE(s1 == #NAME);\
    }

  CHK_TO_STRING(SrvReq, unknown)
  CHK_TO_STRING(SrvReq, read)
  CHK_TO_STRING(SrvReq, write)

  CHK_TO_STRING(TransfMode, unknown)
  CHK_TO_STRING(TransfMode, netascii)
  CHK_TO_STRING(TransfMode, octet)
  CHK_TO_STRING(TransfMode, binary)

  CHK_TO_STRING(LogLvl, emerg)
  CHK_TO_STRING(LogLvl, alert)
  CHK_TO_STRING(LogLvl, crit)
  CHK_TO_STRING(LogLvl, err)
  CHK_TO_STRING(LogLvl, warning)
  CHK_TO_STRING(LogLvl, notice)
  CHK_TO_STRING(LogLvl, info)
  CHK_TO_STRING(LogLvl, debug)

#undef CHK_TO_STRING
//
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
