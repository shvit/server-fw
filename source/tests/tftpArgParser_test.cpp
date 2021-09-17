/**
 * \file tftpArgParser_test.cpp
 * \brief Unit-tests for class ArgParser
 *
 *  License GPL-3.0
 *
 *  \date 10-jun-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
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
  using ArgParser::chk_arg;
  using ArgParser::constr_arg;
  using ArgParser::constr_args;
  using ArgParser::constr_caption;
  using ArgParser::constr_line_out;

  using tftp::ArgParser::ArgParser;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(main, "Check main methods")

// 1
START_ITER("Test chk_arg()");
{
  ArgParser_test p;

  // Short
  {
    auto [r1,r2] = p.chk_arg("-g");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_short);
    TEST_CHECK_TRUE(r2 == "g");
  }
  {
    auto [r1,r2] = p.chk_arg("-A");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_short);
    TEST_CHECK_TRUE(r2 == "A");
  }
  {
    auto [r1,r2] = p.chk_arg("-AB");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_short);
    TEST_CHECK_TRUE(r2 == "AB");
  }

  // Long
  {
    auto [r1,r2] = p.chk_arg("--g");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_long);
    TEST_CHECK_TRUE(r2 == "g");
  }
  {
    auto [r1,r2] = p.chk_arg("--goo");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_long);
    TEST_CHECK_TRUE(r2 == "goo");
  }
  {
    auto [r1,r2] = p.chk_arg("--goo-gle");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::is_long);
    TEST_CHECK_TRUE(r2 == "goo-gle");
  }

  // Normal value
  {
    auto [r1,r2] = p.chk_arg("---");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value);
    TEST_CHECK_TRUE(r2 == "---");
  }
  {
    auto [r1,r2] = p.chk_arg("----");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value);
    TEST_CHECK_TRUE(r2 == "----");
  }
  {
    auto [r1,r2] = p.chk_arg("-----");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value);
    TEST_CHECK_TRUE(r2 == "-----");
  }
  {
    auto [r1,r2] = p.chk_arg("10.10.10.10:1000");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value);
    TEST_CHECK_TRUE(r2 == "10.10.10.10:1000");
  }
  {
    auto [r1,r2] = p.chk_arg("This");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::normal_value);
    TEST_CHECK_TRUE(r2 == "This");
  }

  // End of parse
  {
    auto [r1,r2] = p.chk_arg("--");
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::end_parse);
    TEST_CHECK_TRUE(r2 == "--");
  }

  // NULL
  {
    auto [r1,r2] = p.chk_arg(nullptr);
    TEST_CHECK_TRUE(r1 == ArgParser_test::ArgType::not_found);
    TEST_CHECK_TRUE(r2 == "");
  }
}

// 2
START_ITER("Test constr_arg()");
{
  ArgParser_test p;

  // wrong
  TEST_CHECK_TRUE(p.constr_arg("") == "");
  TEST_CHECK_TRUE(p.constr_arg("-") == "");
  TEST_CHECK_TRUE(p.constr_arg("--") == "");
  TEST_CHECK_TRUE(p.constr_arg("---") == "");
  TEST_CHECK_TRUE(p.constr_arg("----") == "");
  TEST_CHECK_TRUE(p.constr_arg("-----") == "");
  TEST_CHECK_TRUE(p.constr_arg("-A") == "");
  TEST_CHECK_TRUE(p.constr_arg("-AB") == "");
  TEST_CHECK_TRUE(p.constr_arg("-defg") == "");
  TEST_CHECK_TRUE(p.constr_arg("--A") == "");
  TEST_CHECK_TRUE(p.constr_arg("--AB") == "");
  TEST_CHECK_TRUE(p.constr_arg("--defg") == "");
  TEST_CHECK_TRUE(p.constr_arg(" ") == "");
  TEST_CHECK_TRUE(p.constr_arg(" A") == "");
  TEST_CHECK_TRUE(p.constr_arg(" bcd") == "");

  // normal short
  TEST_CHECK_TRUE(p.constr_arg("a") == "-a");
  TEST_CHECK_TRUE(p.constr_arg("Z") == "-Z");

  // normal long
  TEST_CHECK_TRUE(p.constr_arg("ab") == "--ab");
  TEST_CHECK_TRUE(p.constr_arg("abc") == "--abc");
  TEST_CHECK_TRUE(p.constr_arg("Fuck") == "--Fuck");
}

// 3
START_ITER("Test constr_args()");
{
  ArgParser_test p;

  // Empty
  TEST_CHECK_TRUE(p.constr_args({}) == "");
  TEST_CHECK_TRUE(p.constr_args({"","",""}) == "");
  TEST_CHECK_TRUE(p.constr_args({"","-","--","-1","-dfhfgh"}) == "");

  // OK
  TEST_CHECK_TRUE(p.constr_args({"a"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"V"}) == "-V");
  TEST_CHECK_TRUE(p.constr_args({"vers"}) == "--vers");
  TEST_CHECK_TRUE(p.constr_args({"S","vers"}) == "{-S|--vers}");
  TEST_CHECK_TRUE(p.constr_args({"a","biz","def"}) == "{-a|--biz|--def}");
  TEST_CHECK_TRUE(p.constr_args({"a","b","d"}) == "{-a|-b|-d}");

  // Partial
  TEST_CHECK_TRUE(p.constr_args({"","a"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"a",""}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"-","a"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"a","-"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"--","a"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"a","--"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"-X","a"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"a","-X"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"--wer","a"}) == "-a");
  TEST_CHECK_TRUE(p.constr_args({"a","--wer"}) == "-a");

  TEST_CHECK_TRUE(p.constr_args({"","a","-","Exp"}) == "{-a|--Exp}");
  TEST_CHECK_TRUE(p.constr_args({"-F","a","--des","Exp"}) == "{-a|--Exp}");
  TEST_CHECK_TRUE(p.constr_args({"","a","-","Exp","--kek"}) == "{-a|--Exp}");
  TEST_CHECK_TRUE(p.constr_args({"-F","a","--des","Exp"," "}) == "{-a|--Exp}");
}

// 4
START_ITER("Test constr_line_out()");
{
  ArgParser_test p;

  TEST_CHECK_TRUE(p.constr_line_out({
      int{0},
      tftp::VecStr{},
      tftp::ArgExistVaue::required,
      std::string{"file"},
      std::string{"Nothing"},
      std::string{""}}) == "Nothing");

  TEST_CHECK_TRUE(p.constr_line_out({
      int{0},
      tftp::VecStr{"--"},
      tftp::ArgExistVaue::required,
      std::string{"file"},
      std::string{"Nothing"},
      std::string{"other"}}) == "Nothing (other)");

  TEST_CHECK_TRUE(p.constr_line_out({
      int{1},
      tftp::VecStr{"L"},
      tftp::ArgExistVaue::optional,
      std::string{},
      std::string{"Locale"},
      std::string{}}) == "-L [<value>] Locale");

  TEST_CHECK_TRUE(p.constr_line_out({
      int{1},
      tftp::VecStr{"l", "L", "local"},
      tftp::ArgExistVaue::required,
      std::string{"file"},
      std::string{"Local file name"},
      std::string{}}) == "{-l|-L|--local} <file> Local file name");

  TEST_CHECK_TRUE(p.constr_line_out({
      int{1},
      tftp::VecStr{"l", "L"},
      tftp::ArgExistVaue::no,
      std::string{"file"},
      std::string{},
      std::string{}}) == "{-l|-L} ...");

  TEST_CHECK_TRUE(p.constr_line_out({
      int{1},
      tftp::VecStr{},
      tftp::ArgExistVaue::no,
      std::string{"file"},
      std::string{},
      std::string{}}) == "");

  TEST_CHECK_TRUE(p.constr_line_out({
      int{1},
      tftp::VecStr{},
      tftp::ArgExistVaue::no,
      std::string{"file"},
      std::string{},
      std::string{"ANY TEXT"}}) == "(ANY TEXT)");
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(full_test, "Check method go_full()")

size_t log_err{0U};
size_t log_wrn{0U};
size_t log_inf{0U};
size_t log_dbg{0U};

auto log_reset=[&]()
    {
      log_err=0U;
      log_wrn=0U;
      log_inf=0U;
      log_dbg=0U;
    };

auto log_local=[&](tftp::LogLvl lvl, std::string_view msg)
{
  switch(lvl)
  {
    case tftp::LogLvl::err:     ++log_err; break;
    case tftp::LogLvl::warning: ++log_wrn; break;
    case tftp::LogLvl::info:    ++log_inf; break;
    case tftp::LogLvl::debug:   ++log_dbg; break;
    default:  break;
  }
  //std::cout << "[DEBUG] " << tftp::to_string(lvl) << " " <<  msg << std::endl;
};

tftp::ArgItems arg_items
{
  {0, {}, tftp::ArgExistVaue::no, "", "Simple TFTP client from 'server-fw' project licensed GPL-3.0", ""},
  {0, {}, tftp::ArgExistVaue::no, "", "Github project page https://github.com/shvit/server-fw", ""},
  {0, {}, tftp::ArgExistVaue::no, "", "(c) 2019-2021 Vitaliy.V.Shirinkin, e-mail: vitaliy.shirinkin@gmail.com", ""},
  {99,{}, tftp::ArgExistVaue::no, "", "--", ""},
  {0, {}, tftp::ArgExistVaue::no, "", "Usage:", ""},
  {0, {}, tftp::ArgExistVaue::no, "", "./tftp-cl [<options> ... ] <IP addr>[:<Port>]", ""},
  {0, {}, tftp::ArgExistVaue::no, "", "Possible options:", ""},
  { 1, {"l", "L", "local"},      tftp::ArgExistVaue::required, "file", "Local file path and name", ""},
  { 2, {"r", "R", "remote"},     tftp::ArgExistVaue::required, "file", "Remote file name", ""},
  { 3, {"g", "G", "get"},        tftp::ArgExistVaue::no,       "",     "Get file from server", ""},
  { 4, {"p", "P", "put"},        tftp::ArgExistVaue::no,       "",     "Put file to server", ""},
  { 5, {"h", "H", "help", "?"},  tftp::ArgExistVaue::no,       "",     "Show help information", ""},
  { 6, {"v", "V", "verb"},       tftp::ArgExistVaue::no,       "",     "", ""},
  { 7, {"m", "M", "mode"},       tftp::ArgExistVaue::required, "mode", "TFTP transfer mode", ""},
  { 8, {"b", "B", "blksize"},    tftp::ArgExistVaue::required, "N",    "TFTP option 'block size'", "default 512"},
  { 9, {"t", "T", "timeout"},    tftp::ArgExistVaue::required, "N",    "TFTP option 'timeout'", "default 10"},
  {10, {"w", "W", "windowsize"}, tftp::ArgExistVaue::required, "N",    "TFTP option 'windowsize'", "default 1"},
  {11, {"Q","tsize"},            tftp::ArgExistVaue::optional, "N",    "TFTP option 'tsize'", "WRQ without value use calculated"},
  {100,{},                       tftp::ArgExistVaue::required, "",     "", "Testing output"   },
};

ArgParser_test p(arg_items);

START_ITER("Check constr_caption()");
{
  TEST_CHECK_TRUE(p.constr_caption(12345) == "");
  TEST_CHECK_TRUE(p.constr_caption(100) == "Action #100");
  TEST_CHECK_TRUE(p.constr_caption(1) == "Local file path and name");
  TEST_CHECK_TRUE(p.constr_caption(99) == ""); // --
}

START_ITER("Stage 1 - Common check - many doubles");
{
const char * tst_args[]=
{
  "./tftp-cl",
  "B-E-G-I-N",
  "-g", "-G", "--get",
  "-gv",
  "-p", "-P", "--put",
  "-v", "-V", "--verb",
  "-l", // FAIL - need value
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
  "-Z", "--zuuu", // FAIL - unknown arguments
  "-l", // FAIL - need value
  "--",                               // end parsing
  "--local", "test_local4.txt", "-H", // pass as simple values
};

//p.out_help(std::cout); // develop checks
//p.out_header(std::cout); // develop checks

log_reset();

const auto & res = p.run(
    log_local,
    sizeof(tst_args)/sizeof(tst_args[0]),
    const_cast<char **>(tst_args));

TEST_CHECK_TRUE (log_err == 0U);
TEST_CHECK_TRUE (log_wrn == 4U);
TEST_CHECK_TRUE (log_inf == 0U);
TEST_CHECK_TRUE (log_dbg == 40U);

TEST_CHECK_TRUE(res.second.size() == 6U);
if(res.second.size() > 0U) TEST_CHECK_TRUE(res.second[0U] == "B-E-G-I-N");
if(res.second.size() > 1U) TEST_CHECK_TRUE(res.second[1U] == "10.0.0.202:6900");
if(res.second.size() > 2U) TEST_CHECK_TRUE(res.second[2U] == "ending");
if(res.second.size() > 3U) TEST_CHECK_TRUE(res.second[3U] == "--local");
if(res.second.size() > 4U) TEST_CHECK_TRUE(res.second[4U] == "test_local4.txt");
if(res.second.size() > 5U) TEST_CHECK_TRUE(res.second[5U] == "-H");

TEST_CHECK_TRUE(res.first.size() == 11U);

size_t iter_count=0U;
size_t skip_count=0U;
for(const auto & item: res.first)
{
  const auto & id  = item.first;
  const auto & v  = item.second;
  const auto vs = v.size();

  switch(id)
  {
    case 1: // local
      TEST_CHECK_TRUE(vs == 3U);
      if(vs>0U)
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
      ++iter_count;
      break;

    case 2: // remote
      TEST_CHECK_TRUE(vs == 3U);
      if(vs>0U)
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
      ++iter_count;
      break;

    case 3: // get
      TEST_CHECK_TRUE(v.size() == 4U);
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
            if(vs>3U)
            {
              TEST_CHECK_TRUE(v[3U].first == "-g");
              TEST_CHECK_TRUE(v[3U].second == "");
            }
          }
        }
      }
      ++iter_count;
      break;

    case 4: // put
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
      ++iter_count;
      break;

    case 5: // help
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
      ++iter_count;
      break;

    case 6: // verb
      TEST_CHECK_TRUE(v.size() == 4U);
      if(auto vs=v.size(); vs>0U)
      {
        TEST_CHECK_TRUE(v[0U].first == "-v");
        TEST_CHECK_TRUE(v[0U].second == "");
        if(vs>1U)
        {
          TEST_CHECK_TRUE(v[1U].first == "-v");
          TEST_CHECK_TRUE(v[1U].second == "");
          if(vs>2U)
          {
            TEST_CHECK_TRUE(v[2U].first == "-V");
            TEST_CHECK_TRUE(v[2U].second == "");
            if(vs>3U)
            {
              TEST_CHECK_TRUE(v[3U].first == "--verb");
              TEST_CHECK_TRUE(v[3U].second == "");
            }
          }
        }
      }
      ++iter_count;
      break;

    case 7: // mode
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
      ++iter_count;
      break;

    case 8: // blksize
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
      ++iter_count;
      break;

    case 9: // timeout
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
      ++iter_count;
      break;

    case 10: // windowsize
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
      ++iter_count;
      break;

    case 11: // tsize
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
      ++iter_count;
      break;

    default:
      ++skip_count;
      break;
  }
}
TEST_CHECK_TRUE(iter_count == 11U);
TEST_CHECK_TRUE(skip_count == 0U);

TEST_CHECK_TRUE(p.get_parsed_item( 1) == "test_local3.txt");
TEST_CHECK_TRUE(p.get_parsed_item( 2) == "test_remote300.txt");
TEST_CHECK_TRUE(p.get_parsed_item( 3) == "");
TEST_CHECK_TRUE(p.get_parsed_item( 4) == "");
TEST_CHECK_TRUE(p.get_parsed_item( 5) == "");
TEST_CHECK_TRUE(p.get_parsed_item( 6) == "");
TEST_CHECK_TRUE(p.get_parsed_item( 7) == "mail");
TEST_CHECK_TRUE(p.get_parsed_item( 8) == "4096");
TEST_CHECK_TRUE(p.get_parsed_item( 9) == "7");
TEST_CHECK_TRUE(p.get_parsed_item(10) == "27");
TEST_CHECK_TRUE(p.get_parsed_item(11) == "232334345");

  // chk_parsed_item()
  {
    auto [r1,r2] = p.chk_parsed_item(3);
    TEST_CHECK_TRUE(r1==tftp::ResCheck::wrn_many_arg);
    TEST_CHECK_TRUE(r2.size() > 0U); // Warning message
  }
  {
    auto [r1,r2] = p.chk_parsed_item(100);
    TEST_CHECK_TRUE(r1==tftp::ResCheck::not_found);
    TEST_CHECK_TRUE(r2.size() > 0U); // Notify message
  }

  // get_parsed_int()
  {
    auto r = p.get_parsed_int(11);
    TEST_CHECK_TRUE(r.has_value());
    if(r.has_value()) TEST_CHECK_TRUE(r.value() == 232334345);
  }
  {
    auto r = p.get_parsed_int(10);
    TEST_CHECK_TRUE(r.has_value());
    if(r.has_value()) TEST_CHECK_TRUE(r.value() == 27);
  }
  {
    auto r = p.get_parsed_int(9);
    TEST_CHECK_TRUE(r.has_value());
    if(r.has_value()) TEST_CHECK_TRUE(r.value() == 7);
  }
  {
    auto r = p.get_parsed_int(8);
    TEST_CHECK_TRUE(r.has_value());
    if(r.has_value()) TEST_CHECK_TRUE(r.value() == 4096);
  }
  { auto r = p.get_parsed_int(    7); TEST_CHECK_FALSE(r.has_value()); }
  { auto r = p.get_parsed_int(    6); TEST_CHECK_FALSE(r.has_value()); }
  { auto r = p.get_parsed_int(    5); TEST_CHECK_FALSE(r.has_value()); }
  { auto r = p.get_parsed_int(    4); TEST_CHECK_FALSE(r.has_value()); }
  { auto r = p.get_parsed_int(    3); TEST_CHECK_FALSE(r.has_value()); }
  { auto r = p.get_parsed_int(    2); TEST_CHECK_FALSE(r.has_value()); }
  { auto r = p.get_parsed_int(    1); TEST_CHECK_FALSE(r.has_value()); }
  { auto r = p.get_parsed_int(10002); TEST_CHECK_FALSE(r.has_value()); }
}


START_ITER("Stage 2 - Multi short options with 'required'");
{
  const char * tst_args2[]=
  {
    "./tftp-cl",
    "-LR", "file_name", "--get",
    "127.0.0.1",
  };

  const auto & res2 = p.run(
      nullptr,
      sizeof(tst_args2)/sizeof(tst_args2[0]),
      const_cast<char **>(tst_args2));

  TEST_CHECK_TRUE(res2.second.size() == 1U);
  if(res2.second.size() > 0U) TEST_CHECK_TRUE(res2.second[0U] == "127.0.0.1");

  TEST_CHECK_TRUE(res2.first.size() == 3U);


  size_t iter_count=0U;
  size_t skip_count=0U;
  for(const auto & item: res2.first)
  {
    const auto & id  = item.first;
    const auto & v  = item.second;
    const auto vs = v.size();

    switch(id)
    {
      case 1: // local
        TEST_CHECK_TRUE(vs == 1U);
        if(vs>0U)
        {
          TEST_CHECK_TRUE(v[0U].first == "-L");
          TEST_CHECK_TRUE(v[0U].second == "file_name");
        }
        ++iter_count;
        break;

      case 2: // remote
        TEST_CHECK_TRUE(vs == 1U);
        if(vs>0U)
        {
          TEST_CHECK_TRUE(v[0U].first == "-R");
          TEST_CHECK_TRUE(v[0U].second == "file_name");
        }
        ++iter_count;
        break;

      case 3: // get
        TEST_CHECK_TRUE(vs == 1U);
        if(vs>0U)
        {
          TEST_CHECK_TRUE(v[0U].first == "--get");
          TEST_CHECK_TRUE(v[0U].second == "");
        }
        ++iter_count;
        break;

      default:
        ++skip_count;
        break;
    }
  }
  TEST_CHECK_TRUE(iter_count == 3U);
  TEST_CHECK_TRUE(skip_count == 0U);

  TEST_CHECK_TRUE(p.get_parsed_item(1) == "file_name");
  TEST_CHECK_TRUE(p.get_parsed_item(2) == "file_name");
  TEST_CHECK_TRUE(p.get_parsed_item(3) == "");
}

START_ITER("Stage 3 - Multi short options with 'optional' and 'no'");
{
  const char * tst_args2[]=
  {
    "./tftp-cl",
    "-LGQ", "file_name",
    "127.0.0.1",
  };

  const auto & res2 = p.run(
      nullptr,
      sizeof(tst_args2)/sizeof(tst_args2[0]),
      const_cast<char **>(tst_args2));

  TEST_CHECK_TRUE(res2.second.size() == 1U);
  if(res2.second.size() > 0U) TEST_CHECK_TRUE(res2.second[0U] == "127.0.0.1");

  TEST_CHECK_TRUE(res2.first.size() == 3U);

  size_t iter_count=0U;
  size_t skip_count=0U;
  for(const auto & item: res2.first)
  {
    const auto & id  = item.first;
    const auto & v  = item.second;
    const auto vs = v.size();

    switch(id)
    {
      case 1: // local
        TEST_CHECK_TRUE(vs == 1U);
        if(vs>0U)
        {
          TEST_CHECK_TRUE(v[0U].first == "-L");
          TEST_CHECK_TRUE(v[0U].second == "file_name");
        }
        ++iter_count;
        break;

      case 3: // get
        TEST_CHECK_TRUE(vs == 1U);
        if(vs>0U)
        {
          TEST_CHECK_TRUE(v[0U].first == "-G");
          TEST_CHECK_TRUE(v[0U].second == "");
        }
        ++iter_count;
        break;

      case 11: // tsize
        TEST_CHECK_TRUE(vs == 1U);
        if(vs>0U)
        {
          TEST_CHECK_TRUE(v[0U].first == "-Q");
          TEST_CHECK_TRUE(v[0U].second == "file_name");
        }
        ++iter_count;
        break;

      default:
        ++skip_count;
        break;
    }
  }
  TEST_CHECK_TRUE(iter_count == 3U);
  TEST_CHECK_TRUE(skip_count == 0U);

  {
    auto [r1,r2] = p.chk_parsed_item(1);
    TEST_CHECK_TRUE(r1==tftp::ResCheck::normal);
    TEST_CHECK_TRUE(r2.size() == 0U); // No message
  }
  {
    auto [r1,r2] = p.chk_parsed_item(1001);
    TEST_CHECK_TRUE(r1==tftp::ResCheck::err_wrong_data);
    TEST_CHECK_TRUE(r2.size() > 0U); // Error message
  }
  TEST_CHECK_TRUE(p.get_parsed_item( 1) == "file_name");
  TEST_CHECK_TRUE(p.get_parsed_item( 3) == "");
  TEST_CHECK_TRUE(p.get_parsed_item(11) == "file_name");
}

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
