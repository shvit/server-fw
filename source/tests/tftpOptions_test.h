/**
 * \file tftpOptions_test.h
 * \brief Unit-tests for class Options header file
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TESTS_TFTPOPTIONS_TEST_H_
#define SOURCE_TESTS_TFTPOPTIONS_TEST_H_

#include "../tftpOptions.h"

namespace Options
{

//------------------------------------------------------------------------------

/** \brief Helper class for unit-test access to protected field
 */
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

} // namespace Options

#endif /* SOURCE_TESTS_TFTPOPTIONS_TEST_H_ */
