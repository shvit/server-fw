/*
 * tftpArgParser_test.cpp
 *
 *  Created on: 10 июн. 2021 г.
 *      Author: svv
 */




#include "../tftpArgParser.h"
#include "test.h"

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(ArgParse)

//------------------------------------------------------------------------------

class ArgParser_test: public tftp::ArgParser
{
public:
  using ArgParser::ArgType;
  using ArgParser::check_arg;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(main, "Check main methods")

// 1
START_ITER("Test check_arg()");
{
  ArgParser_test p1;

  // Short
  { auto [r1,r2] = p1.check_arg("-g"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_short); TEST_CHECK_TRUE(r2 == "g"); }
  { auto [r1,r2] = p1.check_arg("-A"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_short); TEST_CHECK_TRUE(r2 == "A"); }

  // Long
  { auto [r1,r2] = p1.check_arg("--g"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_long); TEST_CHECK_TRUE(r2 == "g"); }
  { auto [r1,r2] = p1.check_arg("--goo"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_long); TEST_CHECK_TRUE(r2 == "goo"); }
  { auto [r1,r2] = p1.check_arg("--goo-gle"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_long); TEST_CHECK_TRUE(r2 == "goo-gle"); }

  // Normal value
  { auto [r1,r2] = p1.check_arg("--"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value); TEST_CHECK_TRUE(r2 == "--"); }
  { auto [r1,r2] = p1.check_arg("10.10.10.10:1000"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value); TEST_CHECK_TRUE(r2 == "10.10.10.10:1000"); }
  { auto [r1,r2] = p1.check_arg("This"); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value); TEST_CHECK_TRUE(r2 == "This"); }

  // Zero
  { auto [r1,r2] = p1.check_arg(nullptr); TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::not_found); TEST_CHECK_TRUE(r2 == ""); }
}
/*
// 2
START_ITER("Test go()");
{

  tftp::ArgItems arg_items
  {
    { 1, {"l", "L", "local"},      tftp::ArgExistVaue::required, "Local file name"},
    { 2, {"r", "R", "remote"},     tftp::ArgExistVaue::required, "Remote file name"},
    { 3, {"g", "G", "get"},        tftp::ArgExistVaue::no,       "Get file from server"},
    { 4, {"p", "P", "put"},        tftp::ArgExistVaue::no,       "Put file to server"},
    { 5, {"h", "H", "help", "?"},  tftp::ArgExistVaue::no,       "Show help information"},
    { 6, {"v", "V", "verb"},       tftp::ArgExistVaue::no,       "Set verbosity mode"},
    { 7, {"m", "M", "mode"},       tftp::ArgExistVaue::required, "TFTP transfer mode"},
    { 8, {"b", "B", "blksize"},    tftp::ArgExistVaue::required, "TFTP option 'block size' (default 512)"},
    { 9, {"t", "T", "timeout"},    tftp::ArgExistVaue::required, "TFTP option 'timeout' (default 10)"},
    {10, {"w", "W", "windowsize"}, tftp::ArgExistVaue::required, "TFTP option 'windowsize' (default 1)"},
    {11, {"tsize"},                tftp::ArgExistVaue::optional, "TFTP option 'tsize'; without value use calculated for WRQ"},
  };

  const char * tst_args[]=
  {
    "./tftp-cl",
    "B-E-G-I-N",
    "-g", "-G", "--get",
    "-p", "-P", "--put",
    "-v", "-V", "--verb",
    "-l", "test_local.txt", "-L", "test_local.txt", "--local", "test_local.txt",
    "-r", "test_remote.txt", "-R", "test_remote.txt", "--remote", "test_remote.txt",
    "-m", "netascii", "-M", "netascii", "--mode", "netascii",
    "-b", "1300", "-B", "1300", "--blksize", "1300",
    "-t", "20", "-T", "20", "--timeout", "20",
    "-w", "15", "-W", "15", "--windowsize", "15",
    "--tsize", "232334345",
    "10.0.0.202:6900",
    "-h", "-H", "--help", "-?",
    "ending",
  };

  const std::vector<int> res_val1{1,};


  std::map<int, int> counter;
  int err_count = 0;

  auto cb_for_arg = [&](const int & id, std::string val) -> void
      {
        //std::cout << " # id=" << id << " " << val<< std::endl;
        counter[id] = counter[id] + 1;
        switch(id)
        {
          case 1:
            if(val != "test_local.txt") ++err_count;
            break;
          case 2:
            if(val != "test_remote.txt") ++err_count;
            break;
          case 3:
          case 4:
          case 5:
          case 6:
            if(val != "") ++err_count;
            break;
          case 7:
            if(val != "netascii") ++err_count;
            break;
          case 8:
            if(val != "1300") ++err_count;
            break;
          case 9:
            if(val != "20") ++err_count;
            break;
          case 10:
            if(val != "15") ++err_count;
            break;
          case 11:
            if(val != "232334345") ++err_count;
            break;
          default:
            ++err_count;
            break;
        }
      };

  tftp::ArgParser p;

  auto res = p.go(
      arg_items,
      cb_for_arg,
      sizeof(tst_args)/sizeof(tst_args[0]),
      const_cast<char **>(tst_args));

  //std::cout << "----------------------" << std::endl;

  //std::cout << " # res.size()=" << res.size() << std::endl;

  TEST_CHECK_TRUE(res.size() == 3U);
  if(res.size() > 0U) TEST_CHECK_TRUE(res[0U] == "B-E-G-I-N");
  if(res.size() > 1U) TEST_CHECK_TRUE(res[1U] == "10.0.0.202:6900");
  if(res.size() > 2U) TEST_CHECK_TRUE(res[2U] == "ending");

  TEST_CHECK_TRUE(err_count == 0);
  TEST_CHECK_TRUE(counter.size() == 11U);
  TEST_CHECK_TRUE(counter[ 1] == 3);
  TEST_CHECK_TRUE(counter[ 2] == 3);
  TEST_CHECK_TRUE(counter[ 3] == 3);
  TEST_CHECK_TRUE(counter[ 4] == 3);
  TEST_CHECK_TRUE(counter[ 5] == 4);
  TEST_CHECK_TRUE(counter[ 6] == 3);
  TEST_CHECK_TRUE(counter[ 7] == 3);
  TEST_CHECK_TRUE(counter[ 8] == 3);
  TEST_CHECK_TRUE(counter[ 9] == 3);
  TEST_CHECK_TRUE(counter[10] == 3);
  TEST_CHECK_TRUE(counter[11] == 1);
  TEST_CHECK_TRUE(counter[12] == 0);
}
*/
// 3
START_ITER("Test go_full()");
{

  tftp::ArgItems arg_items
  {
    { 1, {"l", "L", "local"},      tftp::ArgExistVaue::required, "Local file name"},
    { 2, {"r", "R", "remote"},     tftp::ArgExistVaue::required, "Remote file name"},
    { 3, {"g", "G", "get"},        tftp::ArgExistVaue::no,       "Get file from server"},
    { 4, {"p", "P", "put"},        tftp::ArgExistVaue::no,       "Put file to server"},
    { 5, {"h", "H", "help", "?"},  tftp::ArgExistVaue::no,       "Show help information"},
    { 6, {"v", "V", "verb"},       tftp::ArgExistVaue::no,       "Set verbosity mode"},
    { 7, {"m", "M", "mode"},       tftp::ArgExistVaue::required, "TFTP transfer mode"},
    { 8, {"b", "B", "blksize"},    tftp::ArgExistVaue::required, "TFTP option 'block size' (default 512)"},
    { 9, {"t", "T", "timeout"},    tftp::ArgExistVaue::required, "TFTP option 'timeout' (default 10)"},
    {10, {"w", "W", "windowsize"}, tftp::ArgExistVaue::required, "TFTP option 'windowsize' (default 1)"},
    {11, {"tsize"},                tftp::ArgExistVaue::optional, "TFTP option 'tsize'; without value use calculated for WRQ"},
  };

  const char * tst_args[]=
  {
    "./tftp-cl",
    "B-E-G-I-N",
    "-g", "-G", "--get",
    //"-gv",
    "-p", "-P", "--put",
    "-v", "-V", "--verb",
    "-l", "test_local1.txt", "-L", "test_local2.txt", "--local", "test_local3.txt",
    "-r", "test_remote100.txt", "-R", "test_remote200.txt", "--remote", "test_remote300.txt",
    "-m", "netascii", "-M", "octet", "--mode", "mail",
    "-b", "1300", "-B", "812", "--blksize", "4096",
    "-t", "20", "-T", "99", "--timeout", "7",
    "-w", "15", "-W", "51", "--windowsize", "27",
    "--tsize", "--tsize", "232334345",
    "10.0.0.202:6900",
    "-h", "-H", "--help", "-?",
    "ending",
  };

  tftp::ArgParser p;

  p.out_help_data(arg_items, std::cout);

  auto res = p.go_full(
      arg_items,
      sizeof(tst_args)/sizeof(tst_args[0]),
      const_cast<char **>(tst_args));

  TEST_CHECK_TRUE(res.second.size() == 3U);
  if(res.second.size() > 0U) TEST_CHECK_TRUE(res.second[0U] == "B-E-G-I-N");
  if(res.second.size() > 1U) TEST_CHECK_TRUE(res.second[1U] == "10.0.0.202:6900");
  if(res.second.size() > 2U) TEST_CHECK_TRUE(res.second[2U] == "ending");

  TEST_CHECK_TRUE(res.first.size() == 11U);

  {
    auto & v = res.first[ 1];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs = v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-l");
      TEST_CHECK_TRUE(v[0U].second == "test_local1.txt");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-L");
        TEST_CHECK_TRUE(v[1U].second == "test_local2.txt");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--local");
          TEST_CHECK_TRUE(v[2U].second == "test_local3.txt");
        }
      }
    }
  }
  {
    auto & v = res.first[ 2];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-r");
      TEST_CHECK_TRUE(v[0U].second == "test_remote100.txt");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-R");
        TEST_CHECK_TRUE(v[1U].second == "test_remote200.txt");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--remote");
          TEST_CHECK_TRUE(v[2U].second == "test_remote300.txt");
        }
      }
    }
  }
  {
    auto & v = res.first[ 3];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-g");
      TEST_CHECK_TRUE(v[0U].second == "");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-G");
        TEST_CHECK_TRUE(v[1U].second == "");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--get");
          TEST_CHECK_TRUE(v[2U].second == "");
        }
      }
    }
  }
  {
    auto & v = res.first[ 4];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-p");
      TEST_CHECK_TRUE(v[0U].second == "");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-P");
        TEST_CHECK_TRUE(v[1U].second == "");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--put");
          TEST_CHECK_TRUE(v[2U].second == "");
        }
      }
    }
  }
  {
    auto & v = res.first[ 5];
    TEST_CHECK_TRUE(v.size() == 4U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-h");
      TEST_CHECK_TRUE(v[0U].second == "");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-H");
        TEST_CHECK_TRUE(v[1U].second == "");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--help");
          TEST_CHECK_TRUE(v[2U].second == "");
          if(vs>3U)
          {
            TEST_CHECK_TRUE(v[3U].first == "-?");
            TEST_CHECK_TRUE(v[3U].second == "");
          }
        }
      }
    }
  }
  {
    auto & v = res.first[ 6];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-v");
      TEST_CHECK_TRUE(v[0U].second == "");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-V");
        TEST_CHECK_TRUE(v[1U].second == "");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--verb");
          TEST_CHECK_TRUE(v[2U].second == "");
        }
      }
    }
  }
  {
    auto & v = res.first[ 7];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-m");
      TEST_CHECK_TRUE(v[0U].second == "netascii");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-M");
        TEST_CHECK_TRUE(v[1U].second == "octet");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--mode");
          TEST_CHECK_TRUE(v[2U].second == "mail");
        }
      }
    }
  }
  {
    auto & v = res.first[ 8];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-b");
      TEST_CHECK_TRUE(v[0U].second == "1300");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-B");
        TEST_CHECK_TRUE(v[1U].second == "812");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--blksize");
          TEST_CHECK_TRUE(v[2U].second == "4096");
        }
      }
    }
  }
  {
    auto & v = res.first[ 9];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-t");
      TEST_CHECK_TRUE(v[0U].second == "20");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-T");
        TEST_CHECK_TRUE(v[1U].second == "99");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--timeout");
          TEST_CHECK_TRUE(v[2U].second == "7");
        }
      }
    }
  }
  {
    auto & v = res.first[10];
    TEST_CHECK_TRUE(v.size() == 3U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "-w");
      TEST_CHECK_TRUE(v[0U].second == "15");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "-W");
        TEST_CHECK_TRUE(v[1U].second == "51");
        if(vs>2U)
        {
          TEST_CHECK_TRUE(v[2U].first == "--windowsize");
          TEST_CHECK_TRUE(v[2U].second == "27");
        }
      }
    }
  }
  {
    auto & v = res.first[11];
    TEST_CHECK_TRUE(v.size() == 2U);
    if(auto vs=v.size(); vs>0U)
    {
      TEST_CHECK_TRUE(v[0U].first == "--tsize");
      TEST_CHECK_TRUE(v[0U].second == "");
      if(vs>1U)
      {
        TEST_CHECK_TRUE(v[1U].first == "--tsize");
        TEST_CHECK_TRUE(v[1U].second == "232334345");
      }
    }
  }
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
