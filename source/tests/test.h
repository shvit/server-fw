/**
 * \file test.h
 * \brief Unit test base help definitions
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 */

#ifndef SOURCE_TESTS_TEST_H_
#define SOURCE_TESTS_TEST_H_

#include <boost/test/unit_test.hpp>
#include <iostream>
#include <openssl/md5.h>
#include <experimental/filesystem>

using namespace std::experimental;

using Path = filesystem::path;

using VecMD5 = std::vector<char[MD5_DIGEST_LENGTH]>;

//------------------------------------------------------------------------------

namespace unit_tests
{
  /// Base temp directory preamble
  constexpr std::string_view local_test_dir ="server_fw_temp";

  /// Sizes of test files generated in unit-tests
  constexpr std::array<size_t, 10U> file_sizes
  {
    0U,
    1U,
    511U,
    512U,
    513U,
    1023U,
    1024U,
    1025U,
    67107840U, // 65535 * 1024
    67108865U, // (65536 * 1024)+1
  };

  /// Unit-tests global counter checks
  extern size_t test_counter_check;

  /// Unit-tests global iteration counter
  extern size_t test_counter_iter;

  extern std::string mainMessage;

  /// Temporary local dirctory for tests
  extern filesystem::path local_dir;

  /// MD5 sum for generated files from 'file_sizes'
  extern VecMD5 file_md5;

  /** \brief Check variable 'local_dir' and fill it
   *
   *  Rule name: local_test_dir + directory separator + Number
   *  \return True if directory created/exist, else false
   */
  bool check_local_directory();

  /** \brief Common fill test data algorithm
   *
   *  \param [in] addr Address of buffer
   *  \param [in] size Data size for fill
   *  \param [in] position Offset at buffer
   *  \param [in] file_id ID of file (number)
   */
  void fill_buffer(
      char * addr,
      const size_t & size,
      const size_t & position,
      const size_t & file_id);

  /** \brief Convert buffer addr
   *
   *  Danger! Buffer size need >= MD5_DIGEST_LENGTH
   *  For nullptr return ""
   *  \peram [in] addr Address of buffer
   *  \return String
   */
  auto md5_as_str(const char * addr) -> std::string;


  // deprecated !
  extern bool tmp_dir_created;
  extern std::string tmp_dir;

  void files_delete();

  bool temp_directory_create();


} 

//------------------------------------------------------------------------------

#define CHECK_INCR (++unit_tests::test_counter_check)

#define BEGIN_ITER START_ITER("");

#define START_ITER(...) \
    {\
      ++unit_tests::test_counter_iter;\
      unit_tests::mainMessage = "" __VA_ARGS__;\
    }


#define MESSAGE_WHEN_FAIL(...) \
      "Iter #"<<unit_tests::test_counter_iter<<"{Checks "<<CHECK_INCR\
      <<":Line "<<__LINE__<<"} : Error when {"<<unit_tests::mainMessage <<\
      (unit_tests::mainMessage.size() && (sizeof("" #__VA_ARGS__ )>1) ? ";" : "") <<\
      "" __VA_ARGS__ << "}"


#define TEST_CHECK_EXPR(L_VALUE, R_VALUE, ...) \
  BOOST_CHECK_MESSAGE(\
      (L_VALUE)==(R_VALUE),\
      MESSAGE_WHEN_FAIL(__VA_ARGS__) " with check '" << #L_VALUE \
      <<"' (need="<<std::boolalpha<<R_VALUE<<")")

#define TEST_CHECK_TRUE(VALUE)  TEST_CHECK_EXPR((VALUE), true,  "");

#define TEST_CHECK_FALSE(VALUE) TEST_CHECK_EXPR((VALUE), false, "");


#define UNIT_TEST_CASE_BEGIN(NAM, ...) BOOST_AUTO_TEST_CASE(Case_##NAM)\
    { \
      unit_tests::test_counter_iter=0;\
      unit_tests::mainMessage = \
      ((sizeof( "" __VA_ARGS__ ) > 1) ? "" __VA_ARGS__ : #NAM);

#define UNIT_TEST_CASE_END \
    }

#define UNIT_TEST_SUITE_BEGIN(NAM) BOOST_AUTO_TEST_SUITE(NAM)

#define UNIT_TEST_SUITE_END BOOST_AUTO_TEST_SUITE_END()

//------------------------------------------------------------------------------

#define ERROR_CLASS_METHOD(E,msg) \
    {\
      throw E(std::string{__PRETTY_FUNCTION__} + ": " + msg);\
    }

#define ERROR_CLASS_METHOD__FORMAT(E,...) \
    {\
      char msg[1024]{0};\
      std::snprintf(msg, sizeof(msg), __VA_ARGS__);\
      ERROR_CLASS_METHOD(E,std::string(msg));\
    }

#define ERROR_FUNCTION__RUNTIME__FORMAT(...) \
    ERROR_FUNCTION__FORMAT(std::runtime_error,__VA_ARGS__)


#define CHK_EXCEPTION__INV_ARG(...) \
  try\
  {\
    __VA_ARGS__;\
    throw std::runtime_error("Need exception when try \"" +\
                             std::string{#__VA_ARGS__}+"\"");\
  }\
  catch (std::invalid_argument & ) { CHECK_INCR; };

//------------------------------------------------------------------------------

#endif /* SOURCE_TESTS_TEST_H_ */
