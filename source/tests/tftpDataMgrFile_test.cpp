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

/** \brief Helper class for access to DataMgr protected fields
 */
class DataMgr_test: public tftp::DataMgrFile
{
public:
  DataMgr_test(
      tftp::fLogMsg logger,
      tftp::fSetError err_setter,
      std::string_view filename,
      std::string root_dir):
          tftp::DataMgrFile(
              logger,
              err_setter) {};

  virtual bool active() const override { return true; };

  virtual bool open() override { return true; };

  virtual auto write(
      tftp::SmBufEx::const_iterator buf_begin,
      tftp::SmBufEx::const_iterator buf_end,
      const size_t & position) -> ssize_t override { return 0; };

  virtual auto read(
      tftp::SmBufEx::iterator buf_begin,
      tftp::SmBufEx::iterator buf_end,
      const size_t & position) -> ssize_t override { return 0; };

  virtual void close() override {};

  virtual void cancel() override {};

  using tftp::DataMgrFile::match_md5;
  using tftp::DataMgrFile::active;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(file_write_read, "Check classes DataMgrFile*")

// Prepare
TEST_CHECK_TRUE(check_local_directory());
//std::cout << " * dir=" << local_dir.string() << std::endl;

auto get_dir_name = [&](const size_t & it) -> std::string { return "dir_"+std::to_string(it+1); };

auto get_file_name = [&](const size_t & it) -> std::string { return "file_"+std::to_string(it+1); };

auto get_md5_name = [&](const size_t & it) -> std::string { return get_file_name(it)+".md5"; };

constexpr size_t block=512U;
std::vector<char> buff(block);
std::vector<char> buff2(block);


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
      nullptr,
      nullptr,
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
  }

  // 2 - create file with data and normal close
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
  }

  // 3 - try create file when hie exist (error return)
  TEST_CHECK_FALSE(dm->open());
  TEST_CHECK_FALSE(dm->active());

  // 4 - create file .md5
  auto dm2 = tftp::DataMgrFileWrite::create(
      nullptr,
      nullptr,
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
  }
}

// STAGE 2 - Read all files
for(size_t iter=0; iter < file_sizes.size(); ++iter)
{
  auto dm = tftp::DataMgrFileRead::create(
      nullptr,
      nullptr,
      get_file_name(iter),
      local_dir.string(),
      {});

  TEST_CHECK_FALSE(dm->active());

  // 1 - search and read exist files by his name
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

  // search and read exist files by his MD5

  std::cout << " * MD5=" << md5_as_str(&file_md5[iter][0]) << std::endl;

  auto dm2 = tftp::DataMgrFileRead::create(
      nullptr,
      nullptr,
      md5_as_str(&file_md5[iter][0]),
      local_dir.string(),
      {});
  TEST_CHECK_TRUE(dm2->open());
  TEST_CHECK_TRUE(dm2->active());

  // 3 - search NOT exist files by his unknown name
  auto dm3 = tftp::DataMgrFileRead::create(
      nullptr,
      nullptr,
      "no_"+get_file_name(iter),
      local_dir.string(),
      {});
  TEST_CHECK_FALSE(dm3->open());
  TEST_CHECK_FALSE(dm3->active());

}

// delete temporary files
unit_tests::files_delete();

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
