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

#include <boost/test/unit_test.hpp>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <openssl/md5.h>

#include "test.h"

namespace unit_tests{
  size_t test_counter_iter=0;  ///< Unit test iteration counter
  size_t test_counter_check=0; ///< Unit test check counter
  std::string mainMessage(""); ///< Message in "Error when {...}"

  // TFTP tests helper

  bool tmp_dir_created=false;
  std::string tmp_dir{"test_directory_XXXXXX"};
  std::vector<size_t> file_sizes{0U,1U,511U,512U,513U,1023U,1024U,1025U,65535U*512U*2U};
  std::vector<char[MD5_DIGEST_LENGTH]> file_md5(file_sizes.size());


  void files_delete()
  {
    // files
    for(size_t iter=0; iter < sizeof(file_sizes)/sizeof(file_sizes[0]); ++iter)
    {
      std::string curr_file_name{tmp_dir};
      curr_file_name.append("/file").append(std::to_string(iter+1)).append("\0");
      unlink(curr_file_name.c_str());
      curr_file_name.assign(tmp_dir).append("/file").append(std::to_string(iter + 1)).append(".md5\0");
      unlink(curr_file_name.c_str());
    }

    // directory
    if(tmp_dir_created) TEST_CHECK_TRUE(rmdir(tmp_dir.c_str()) == 0);
  }

  void fill_buffer(char * addr, size_t size, size_t position, size_t file_id)
  {
    for(size_t iter=0; iter < size; ++iter)
    {
      *(addr + iter) = static_cast<uint8_t>((position+iter+file_id)  & 0x000000000000FF);
    }
  }

  std::string md5_as_str(const char * addr)
  {
    std::stringstream ss;
    for(size_t iter=0; iter<MD5_DIGEST_LENGTH; ++iter)
    {
      ss << std::hex << std::setw(2) << std::setfill('0') << (((uint16_t) *(addr+iter)) & 0x00FFU);
    }
    return ss.str();
  }

  bool temp_directory_create()
  {
    if(!tmp_dir_created) (tmp_dir_created = (mkdtemp(const_cast<char*>(tmp_dir.c_str())) != nullptr));
    return tmp_dir_created;
  }


}

using namespace unit_tests;


UNIT_TEST_SUITE_BEGIN(MainTest)

/** \brief Show finish counter
 *
 */
UNIT_TEST_CASE_BEGIN(Finish, "Counter")
  // finalize
  //files_delete();

  // show counter
  std::cout << "Summary checks " << unit_tests::test_counter_check << std::endl;
UNIT_TEST_CASE_END

UNIT_TEST_SUITE_END
