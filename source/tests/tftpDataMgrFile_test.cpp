/**
 * \file tftpDataMgrFile_test.cpp
 * \brief Unit-tests for class DataMgrFile, DataMgrFileRead, DataMgrFileWrite
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include "test.h"
#include "../tftpDataMgrFile.h"
#include "../tftpDataMgrFileRead.h"
#include "../tftpDataMgrFileWrite.h"
#include "tftpOptions_test.h"

UNIT_TEST_SUITE_BEGIN(DataMgrFile)

using namespace unit_tests;

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(file_write_read, "Check classes DataMgrFile*")

// Prepare
TEST_CHECK_TRUE(check_local_directory());

auto get_dir_name = [&](const size_t & it) -> std::string { return "dir_"+std::to_string(it+1); };

auto get_file_name = [&](const size_t & it) -> std::string { return "file_"+std::to_string(it+1); };

auto get_md5_name = [&](const size_t & it) -> std::string { return get_file_name(it)+".md5"; };

constexpr size_t block=512U;
std::vector<char> buff(block);
std::vector<char> buff2(block);

FakeLog<7> fl;
auto cb_syslog = std::bind(
    & FakeLog<7>::syslog,
    & fl,
    std::placeholders::_1,
    std::placeholders::_2);

FakeError fe;
auto cb_seterr = std::bind(
    & FakeError::set_error,
    & fe,
    std::placeholders::_1,
    std::placeholders::_2);

// STAGE 1 - Create all files
for(size_t iter=0; iter < file_sizes.size(); ++iter)
{
  auto curr_dir = local_dir;
  curr_dir /= get_dir_name(iter);

  TEST_CHECK_FALSE(filesystem::exists(curr_dir));
  TEST_CHECK_TRUE(filesystem::create_directories(curr_dir));
}

for(size_t iter=0; iter < file_sizes.size(); ++iter)
{
  bool success_file_rx=true;

  auto curr_dir = local_dir;
  curr_dir /= get_dir_name(iter);

  auto dm = tftp::DataMgrFileWrite::create(
      cb_syslog,
      cb_seterr,
      get_file_name(iter),
      curr_dir.string());

  TEST_CHECK_FALSE(dm->active());

  // 1 - create file with data and cancel (delete) his
  TEST_CHECK_TRUE(dm->open());
  TEST_CHECK_TRUE(dm->active());
  if(dm->active())
  {
    for(size_t stage=0; stage*block < file_sizes[iter]; ++stage)
    {
      size_t left_size = file_sizes[iter] - stage*block;
      if(left_size > block) left_size = block;

      fill_buffer(buff.data(), left_size, stage*block, iter);

      success_file_rx =
          success_file_rx &&
          (dm->write(buff.begin(),
                 buff.begin()+left_size,
                 stage*block) >= 0);
    }
    TEST_CHECK_TRUE(success_file_rx);

    TEST_CHECK_TRUE(dm->active());
    TEST_CHECK_TRUE(filesystem::exists(dm->get_filename()));

    dm->cancel();
    TEST_CHECK_FALSE(dm->active());
    TEST_CHECK_FALSE(filesystem::exists(dm->get_filename()));
    //fl.show();
    TEST_CHECK_TRUE(fl.chk({0U,0U,iter,0U,0U,1U+4U*iter,2U*iter}));
    //fe.show();
    TEST_CHECK_TRUE((fe.code()==(iter==0U?0U:6U)) && (fe.was_error()==iter));
  }

  // 2 - create file with data and normal close
  success_file_rx=true;
  TEST_CHECK_TRUE(dm->open());
  TEST_CHECK_TRUE(dm->active());
  if(dm->active())
  {
    for(size_t stage=0; stage*block < file_sizes[iter]; ++stage)
    {
      size_t left_size = file_sizes[iter] - stage*block;
      if(left_size > block) left_size = block;

      fill_buffer(buff.data(), left_size, stage*block, iter);

      success_file_rx =
          success_file_rx &&
          (dm->write(buff.begin(),
                 buff.begin()+left_size,
                 stage*block) >= 0);
    }
    TEST_CHECK_TRUE(success_file_rx);

    TEST_CHECK_TRUE(dm->active());
    TEST_CHECK_TRUE(filesystem::exists(dm->get_filename()));

    dm->close();
    TEST_CHECK_FALSE(dm->active());
    TEST_CHECK_TRUE(filesystem::exists(dm->get_filename()));

    TEST_CHECK_TRUE(filesystem::file_size(dm->get_filename()) == file_sizes[iter]);

    //fl.show();
    TEST_CHECK_TRUE(fl.chk({0U,0U,iter,0U,0U,2U+4U*iter,1U+2U*iter}));
    //fe.show();
    TEST_CHECK_TRUE((fe.code()==(iter==0U?0U:6U)) && (fe.was_error()==iter));
  }

  // 3 - try create file when hie exist (error return)
  TEST_CHECK_FALSE(dm->open());
  TEST_CHECK_FALSE(dm->active());

  // 4 - create file .md5
  auto dm2 = tftp::DataMgrFileWrite::create(
      cb_syslog, //
      cb_seterr,
      get_md5_name(iter),
      curr_dir.string());

  TEST_CHECK_TRUE(dm2->open());
  TEST_CHECK_TRUE(dm2->active());

  if(dm2->active())
  {
    std::vector<char> buff_data(file_sizes[iter], 0);
    fill_buffer(buff_data.data(), file_sizes[iter], 0, iter);
    MD5((unsigned char*) buff_data.data(),
        file_sizes[iter],
        (unsigned char *) & file_md5[iter][0]);

    std::string md5_file{md5_as_str(& file_md5[iter][0])};
    md5_file.append(" ").append(get_file_name(iter));

    TEST_CHECK_TRUE(dm2->write(
        static_cast<tftp::Buf::iterator>(& *md5_file.begin()),
        static_cast<tftp::Buf::iterator>(& *md5_file.end()),
        0) >= 0);

    TEST_CHECK_TRUE(dm2->active());
    dm2->close();
    TEST_CHECK_FALSE(dm2->active());

    //fl.show();
    TEST_CHECK_TRUE(fl.chk({0U,0U,1U+iter,0U,0U,4U*(iter+1U),2U*(iter+1U)}));
    //fe.show();
    TEST_CHECK_TRUE((fe.code()==6U) && (fe.was_error()==(iter+1U)));
  }
}

// STAGE 2 - Read all files
fl.clear();
fe.clear();
for(size_t iter=0; iter < file_sizes.size(); ++iter)
{
  auto dm = tftp::DataMgrFileRead::create(
      cb_syslog,
      cb_seterr,
      get_file_name(iter),
      local_dir.string(),
      {});

  TEST_CHECK_FALSE(dm->active());

  // 2.1 - search and read exist files by his name
  TEST_CHECK_TRUE(dm->open());
  TEST_CHECK_TRUE(dm->active());
  if(dm->active())
  {
    for(size_t stage = 0; stage * block < file_sizes[iter]; ++stage)
    {
      size_t left_size = file_sizes[iter] - stage * block;
      if(left_size > block) left_size = block;

      fill_buffer(buff.data(), left_size, stage * block, iter);

      TEST_CHECK_TRUE(dm->read(buff2.begin(),
                               buff2.begin()+left_size,
                               stage * block) == (ssize_t)left_size);

      TEST_CHECK_TRUE(std::equal(buff.cbegin(),
                                 buff.cbegin() + left_size,
                                 buff2.cbegin()));
    }

    TEST_CHECK_TRUE(dm->active());
    dm->close();
    TEST_CHECK_FALSE(dm->active());
  }
  //fl.show();
  TEST_CHECK_TRUE(fl.chk({0U,0U,0U,0U,0U,1U,1U+file_sizes[iter]/block +(file_sizes[iter]%block >0U ? 1U : 0U)}));
  //fe.show();
  TEST_CHECK_TRUE((fe.code()==0U) && (fe.was_error()==0U));

  // 2.2 search and read exist files by his MD5
  std::string filename_md5 = md5_as_str(&file_md5[iter][0]);
  fl.clear();
  auto dm2 = tftp::DataMgrFileRead::create(
      cb_syslog,
      cb_seterr,
      filename_md5,
      local_dir.string(),
      {});
  TEST_CHECK_TRUE(dm2->open());
  TEST_CHECK_TRUE(dm2->active());
  //fl.show();
  TEST_CHECK_TRUE(fl.chk({0U,0U,0U,0U,0U,1U,1U}));
  //fe.show();
  TEST_CHECK_TRUE((fe.code()==0U) && (fe.was_error()==0U));

  // 2.3 - search NOT exist files by his unknown name
  auto dm3 = tftp::DataMgrFileRead::create(
      cb_syslog,
      cb_seterr,
      "no_"+get_file_name(iter),
      local_dir.string(),
      {});
  TEST_CHECK_FALSE(dm3->open());
  TEST_CHECK_FALSE(dm3->active());
  //fl.show();
  TEST_CHECK_TRUE(fl.chk({0U,0U,1U,0U,0U,2U,1U}));
  //fe.show();
  TEST_CHECK_TRUE((fe.code()==1U) && (fe.was_error()==1U));
  fl.clear();
  fe.clear();
}

// delete temporary files
unit_tests::files_delete();

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
