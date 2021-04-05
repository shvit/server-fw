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
#include <cstdlib>  // size_t
#include <iostream>
#include <string>
#include <openssl/md5.h>


namespace unit_tests
{
  extern size_t test_counter_iter;
  extern size_t test_counter_check;
  extern std::string mainMessage;

  extern bool tmp_dir_created;
  extern std::string tmp_dir;
  extern std::vector<size_t> file_sizes;
  extern std::vector<char[MD5_DIGEST_LENGTH]> file_md5;

  void files_delete();
  bool temp_directory_create();
  void fill_buffer(char * addr, size_t size, size_t position, size_t file_id);

  std::string md5_as_str(const char * addr);


} 

#define   CHECK_INCR (++unit_tests::test_counter_check)

#define   BEGIN_ITER START_ITER("");
#define   START_ITER(...) { ++unit_tests::test_counter_iter; unit_tests::mainMessage = "" __VA_ARGS__;}

#define MESSAGE_WHEN_FAIL(...) \
      "Iter #"<<unit_tests::test_counter_iter<<"{Checks "<<CHECK_INCR<<":Line "<<__LINE__<<"} "\
      ": Error when {"<<unit_tests::mainMessage << \
      (unit_tests::mainMessage.size() && (sizeof("" #__VA_ARGS__ )>1) ? ";" : "") << "" __VA_ARGS__ << "}"\


#define TEST_CHECK_EXPR(L_VALUE, R_VALUE, ...) \
  BOOST_CHECK_MESSAGE( (L_VALUE)==(R_VALUE),\
                       MESSAGE_WHEN_FAIL(__VA_ARGS__) " with check '" << #L_VALUE <<"' (need="<<std::boolalpha<<R_VALUE<<")"\
                     );

#define TEST_CHECK_NOEQ(L_VALUE, R_VALUE, ...) \
  BOOST_CHECK_MESSAGE( (L_VALUE)!=(R_VALUE),\
                       MESSAGE_WHEN_FAIL(__VA_ARGS__) " with check '" << #L_VALUE <<"' (need="<<std::boolalpha<<R_VALUE<<")"\
                     );


#define TEST_CHECK_VALUE(CLASS, L_VALUE, R_VALUE, ...) \
  BOOST_CHECK_MESSAGE( CLASS.L_VALUE==R_VALUE,\
                       MESSAGE_WHEN_FAIL(__VA_ARGS__) " with check '" << #L_VALUE <<"' (need="<<std::boolalpha<<R_VALUE<<");"\
                      "DEBUG " << CLASS.debugGetCurrentState()\
  );


#define TEST_CHECK_TRUE(VALUE)  TEST_CHECK_EXPR((VALUE), true,  "");
#define TEST_CHECK_FALSE(VALUE) TEST_CHECK_EXPR((VALUE), false, "");


#define UNIT_TEST_CASE_BEGIN(NAM, ...) BOOST_AUTO_TEST_CASE(Case_##NAM)\
    { \
      unit_tests::test_counter_iter=0;\
      unit_tests::mainMessage = ((sizeof( "" __VA_ARGS__ ) > 1) ? "" __VA_ARGS__ : #NAM);

#define UNIT_TEST_CASE_END }

#define UNIT_TEST_SUITE_BEGIN(NAM) BOOST_AUTO_TEST_SUITE(NAM)

#define UNIT_TEST_SUITE_END BOOST_AUTO_TEST_SUITE_END()

//---------------------------------------------------------------------

#define ERROR_CLASS_METHOD(E,msg) { throw E(std::string{__PRETTY_FUNCTION__} + ": " + msg); }

#define ERROR_CLASS_METHOD__FORMAT(E,...)   {char msg[1024]{0}; std::snprintf(msg, sizeof(msg), __VA_ARGS__); ERROR_CLASS_METHOD(E,std::string(msg)); }

#define ERROR_FUNCTION__RUNTIME__FORMAT(...)  ERROR_FUNCTION__FORMAT(std::runtime_error,__VA_ARGS__)


#define CHK_EXCEPTION__INV_ARG(...) \
  try\
  {\
    __VA_ARGS__;\
    throw std::runtime_error("Need exception when try \"" + std::string{#__VA_ARGS__}+"\"");\
  }\
  catch (std::invalid_argument & ) { CHECK_INCR; };

#endif /* SOURCE_TESTS_TEST_H_ */
