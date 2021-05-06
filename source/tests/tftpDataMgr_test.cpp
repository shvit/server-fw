#include "../tftpDataMgr.h"
#include "test.h"     

UNIT_TEST_SUITE_BEGIN(DataMgr)

using namespace unit_tests;

class test_data_mgr: public tftp::DataMgr
{
public:

  using tftp::DataMgr::settings_;
  using tftp::DataMgr::match_md5;
  using tftp::DataMgr::active_files;
  using tftp::DataMgr::active;
};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(md5_check, "check match_md5()")

  TEST_CHECK_FALSE(test_data_mgr{}.match_md5("server_fw"));
  TEST_CHECK_FALSE(test_data_mgr{}.match_md5("server_fw.md5"));
  TEST_CHECK_FALSE(test_data_mgr{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4z"));
  TEST_CHECK_FALSE(test_data_mgr{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4z.md5"));
  TEST_CHECK_FALSE(test_data_mgr{}.match_md5("2fdf093688bb7cef7c05b 1ffcc71ff4e"));
  TEST_CHECK_FALSE(test_data_mgr{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4e.md5"));
  TEST_CHECK_FALSE(test_data_mgr{}.match_md5("172775dbdee46e00a422235475244db6.md5"));
  TEST_CHECK_FALSE(test_data_mgr{}.match_md5(""));

  TEST_CHECK_TRUE (test_data_mgr{}.match_md5("2fdf093688bb7cef7c05b1ffcc71ff4e"));
  TEST_CHECK_TRUE (test_data_mgr{}.match_md5("172775dbdee46e00a422235475244db6"));
  TEST_CHECK_TRUE (test_data_mgr{}.match_md5("00000000000000000000000000000000"));
  TEST_CHECK_TRUE (test_data_mgr{}.match_md5("FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"));

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(files_check, "check with files operations")

// Prepare
TEST_CHECK_TRUE(check_local_directory());

// RX
START_ITER("rx() check - store data files");
{
  for(size_t iter=0; iter < file_sizes.size(); ++iter)
  {
    bool success_file_rx=true;

    std::string curr_file_name = "file" + std::to_string(iter+1);

    test_data_mgr dm;
    dm.settings_->root_dir.assign(local_dir.string());
    TEST_CHECK_FALSE(dm.active_files());
    TEST_CHECK_FALSE(dm.active());

    // 1 - create file with data
    bool init_res;

    TEST_CHECK_TRUE(init_res=dm.init(
        tftp::SrvReq::write,
        curr_file_name));

    if(init_res)
    {
      size_t block=512;
      std::vector<char> buff(block, 0);

      for(size_t stage=0; stage*block < file_sizes[iter]; ++stage)
      {
        size_t left_size = file_sizes[iter] - stage*block;
        if(left_size > block) left_size = block;

        fill_buffer(buff.data(), left_size, stage*block, iter);

        success_file_rx =
            success_file_rx &&
            (dm.rx(buff.begin(),
                   buff.begin()+left_size,
                   stage*block) == 0);
      }
      TEST_CHECK_TRUE(success_file_rx);

      TEST_CHECK_TRUE(dm.active_files());
      TEST_CHECK_TRUE(dm.active());
      dm.close();

      TEST_CHECK_FALSE(dm.active_files());
      TEST_CHECK_FALSE(dm.active());
    }

    // 2 - create file .md5
    TEST_CHECK_TRUE(init_res = dm.init(
        tftp::SrvReq::write,
        std::string{curr_file_name}+".md5"));

    if(init_res)
    {
      std::vector<char> buff_data(file_sizes[iter], 0);
      fill_buffer(buff_data.data(), file_sizes[iter], 0, iter);
      MD5((unsigned char*) buff_data.data(),
          file_sizes[iter],
          (unsigned char *) & file_md5[iter][0]);

      std::string md5_file{md5_as_str(& file_md5[iter][0])};
      md5_file.append(" ").append(curr_file_name);

      TEST_CHECK_TRUE(dm.rx(
          static_cast<tftp::Buf::iterator>(& *md5_file.begin()),
          static_cast<tftp::Buf::iterator>(& *md5_file.end()),
          0) == 0);

      TEST_CHECK_TRUE(dm.active_files());
      TEST_CHECK_TRUE(dm.active());
      dm.close();
      TEST_CHECK_FALSE(dm.active_files());
      TEST_CHECK_FALSE(dm.active());
    }

    // 3 - find file by md5 in root server directory
    std::string hash{md5_as_str(& file_md5[iter][0])};
    test_data_mgr dm_find;
    dm_find.settings_->root_dir.assign(local_dir);
    dm_find.settings_->backup_dirs.push_back(local_dir);
    TEST_CHECK_TRUE(init_res = dm_find.init(tftp::SrvReq::read, hash));
  }
}

// 2.1 TX
START_ITER("tx() check - read data files by name");
{
  for(size_t iter=0; iter < file_sizes.size(); ++iter)
  {
    bool success_tx=true;

    std::string curr_file_name{"file"};
    curr_file_name.append(std::to_string(iter+1));

    test_data_mgr dm;
    dm.settings_->root_dir.assign(local_dir);
    TEST_CHECK_FALSE(dm.active_files());
    TEST_CHECK_FALSE(dm.active());

    bool init_res;
    TEST_CHECK_TRUE(init_res = dm.init(tftp::SrvReq::read, curr_file_name));

    if(init_res)
    {
      size_t block=512;
      std::vector<char> buff_ethalon(block, 0);
      std::vector<char> buff_checked(block, 0);

      for(size_t stage = 0; stage * block < file_sizes[iter]; ++stage)
      {
        size_t left_size = file_sizes[iter] - stage * block;
        if(left_size > block) left_size = block;

        fill_buffer(buff_ethalon.data(), left_size, stage * block, iter);

        success_tx = success_tx && (
            dm.tx(buff_checked.begin(),
                  buff_checked.begin()+left_size,
                  stage * block) == (ssize_t)left_size);

        for(size_t chk_pos = 0; chk_pos < left_size; ++chk_pos)
        {
          success_tx = success_tx &&
              (buff_ethalon[chk_pos] == buff_checked[chk_pos]);
        }
      }

      TEST_CHECK_TRUE(dm.active_files());
      TEST_CHECK_TRUE(dm.active());
      dm.close();
      TEST_CHECK_FALSE(dm.active_files());
      TEST_CHECK_FALSE(dm.active());
      TEST_CHECK_TRUE(success_tx);
    }
  }
}

// 2.2 TX
START_ITER("tx() check - read data files by md5");
{
  for(size_t iter=0; iter < file_sizes.size(); ++iter)
  {
    bool success_tx=true;

    std::string curr_file_name{md5_as_str(& file_md5[iter][0])};

    test_data_mgr dm;
    dm.settings_->root_dir.assign(local_dir);
    TEST_CHECK_FALSE(dm.active_files());
    TEST_CHECK_FALSE(dm.active());

    bool init_res;
    TEST_CHECK_TRUE(init_res = dm.init(tftp::SrvReq::read, curr_file_name));

    if(init_res)
    {
      size_t block=512;
      std::vector<char> buff_ethalon(block, 0);
      std::vector<char> buff_checked(block, 0);

      for(size_t stage = 0; stage * block < file_sizes[iter]; ++stage)
      {
        size_t left_size = file_sizes[iter] - stage * block;
        if(left_size > block) left_size = block;

        fill_buffer(buff_ethalon.data(), left_size, stage * block, iter);

        success_tx = success_tx && (
            dm.tx(buff_checked.begin(),
                  buff_checked.begin()+left_size,
                  stage * block) == (ssize_t)left_size);

        for(size_t chk_pos = 0; chk_pos < left_size; ++chk_pos)
        {
          success_tx = success_tx &&
              (buff_ethalon[chk_pos] == buff_checked[chk_pos]);
        }
      }

      TEST_CHECK_TRUE(dm.active_files());
      TEST_CHECK_TRUE(dm.active());
      dm.close();
      TEST_CHECK_FALSE(dm.active_files());
      TEST_CHECK_FALSE(dm.active());
      TEST_CHECK_TRUE(success_tx);
    }
  }
}

// delete temporary files
unit_tests::files_delete();

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
