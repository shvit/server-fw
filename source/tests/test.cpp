/**
 * \file test.cpp
 * \brief Unit test base help definitions
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE TFTP_TESTS

#include <iomanip>
#include "test.h"

namespace unit_tests
{
  size_t test_counter_iter=0U;

  size_t test_counter_check=0U;

  std::string mainMessage("");

  filesystem::path local_dir;

  VecMD5 file_md5(file_sizes.size());

// -----------------------------------------------------------------------------

bool check_local_directory()
{
  local_dir = filesystem::temp_directory_path();
  local_dir /= local_test_dir;

  if(!filesystem::exists(local_dir))
    if(!filesystem::create_directories(local_dir)) return false;

  size_t iter=1U;
  decltype(local_dir) curr;
  do
  {
    curr = local_dir;
    curr /= std::to_string(iter++);
  }
  while(filesystem::exists(curr));

  local_dir = curr;

  if(filesystem::create_directories(local_dir)) return true;

  throw std::runtime_error("Can't create local temporary directory");
}

// -----------------------------------------------------------------------------

 void files_delete()
{
  if(filesystem::exists(local_dir))
  {
    filesystem::remove_all(local_dir);
  }
}

// -----------------------------------------------------------------------------

void fill_buffer(
    char * addr,
    const size_t & size,
    const size_t & position,
    const size_t & file_id)
{
  for(size_t iter=0; iter < size; ++iter)
  {
    *(addr + iter) = static_cast<uint8_t>((position+iter+file_id)  & 0xFFUL);
  }
}

// -----------------------------------------------------------------------------

auto md5_as_str(const char * addr) -> std::string
{
  std::stringstream ss;

  if(addr != nullptr)
  {
    for(size_t iter=0; iter<MD5_DIGEST_LENGTH; ++iter)
    {
      ss << std::hex << std::setw(2) << std::setfill('0');
      ss << (((uint16_t) *(addr+iter)) & 0x00FFU);
    }
  }
  return ss.str();
}

} // namespace unit_tests

// -----------------------------------------------------------------------------

UNIT_TEST_SUITE_BEGIN(MainTest)

/** \brief Show finish counter
 *
 */
UNIT_TEST_CASE_BEGIN(Finish, "Counter")

  // show counter
  std::cout << "Summary checks " << unit_tests::test_counter_check << std::endl;

UNIT_TEST_CASE_END

UNIT_TEST_SUITE_END

// -----------------------------------------------------------------------------
