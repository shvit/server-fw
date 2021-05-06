#include "../tftpDataMgr.h"
#include "test.h"     

UNIT_TEST_SUITE_BEGIN(DataMgr)

using namespace unit_tests;

class test_data_mgr: public tftp::DataMgr
{
public:

  using tftp::DataMgr::request_name_;
  using tftp::DataMgr::settings_;

  using tftp::DataMgr::check_root_dir;
  using tftp::DataMgr::active_files;
  using tftp::DataMgr::active;
  using tftp::DataMgr::is_md5;
  using tftp::DataMgr::recursive_search_by_md5;

};

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(md5_check, "check is_md5()")

const struct
  {
    const char * name;
    int    mode_md5;
  }
  tst1_item[]=
    {
      {"server_fw",                            0},
      {"server_fw.md5",                        0},
      {"2fdf093688bb7cef7c05b1ffcc71ff4z",     0},
      {"2fdf093688bb7cef7c05b1ffcc71ff4z.md5", 0},
      {"2fdf093688bb7cef7c05b 1ffcc71ff4e",    0},
      {"2fdf093688bb7cef7c05b1ffcc71ff4e",     1},
      {"172775dbdee46e00a422235475244db6",     1},
      {"2fdf093688bb7cef7c05b1ffcc71ff4e.md5", 2},
      {"172775dbdee46e00a422235475244db6.md5", 2},
    };

  START_ITER("is_md5() check on data set");
  {
    for(size_t struct_iter=0;
        struct_iter < sizeof(tst1_item)/sizeof(tst1_item[0]);
        ++struct_iter)
    {
      test_data_mgr dm;
      dm.request_name_.assign(tst1_item[struct_iter].name);
      TEST_CHECK_TRUE(dm.is_md5() == tst1_item[struct_iter].mode_md5);
    }
  }

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(main, "rx_tx")

  // 0 prepare
  START_ITER("prepare - cretae temporary directory");
  {
    TEST_CHECK_TRUE(temp_directory_create());
  }

  // 1 RX data
  START_ITER("rx() check - store data files");
  {
    for(size_t iter=0; iter < file_sizes.size(); ++iter)
    {
      bool success_file_rx=true;

      std::string curr_file_name{};
      curr_file_name.append("file").append(std::to_string(iter+1));

      test_data_mgr dm;
      TEST_CHECK_FALSE(dm.check_root_dir());
      dm.settings_->root_dir.assign(tmp_dir);
      TEST_CHECK_TRUE(dm.check_root_dir());
      TEST_CHECK_FALSE(dm.active_files());
      TEST_CHECK_FALSE(dm.active());

      bool init_res;

      // 1 - file with data create
      TEST_CHECK_TRUE(init_res = dm.init(
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

      // 2 - file md5 create
      TEST_CHECK_TRUE(init_res = dm.init(
          tftp::SrvReq::write,
          curr_file_name+".md5"));

      if(init_res)
      {
        std::vector<char> buff_data(file_sizes[iter], 0);
        fill_buffer(buff_data.data(), file_sizes[iter], 0, iter);
        MD5((unsigned char*) buff_data.data(),
            file_sizes[iter],
            (unsigned char *) & file_md5[iter][0]); //  hash.data()

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
      dm_find.settings_->root_dir.assign(tmp_dir);
      dm_find.settings_->backup_dirs.push_back(tmp_dir);
      TEST_CHECK_TRUE(init_res = dm_find.init(tftp::SrvReq::read, hash));

      if(init_res)
      {
        TEST_CHECK_TRUE(dm_find.request_name_ ==
            dm_find.get_root_dir()+curr_file_name);
      }

    }
  }

  // 2 TX
  START_ITER("tx() check - read data files");
  {
    for(size_t iter=0; iter < file_sizes.size(); ++iter)
    {
      bool success_tx=true;

      std::string curr_file_name{};
      curr_file_name.append("file").append(std::to_string(iter+1));

      test_data_mgr dm;
      TEST_CHECK_FALSE(dm.check_root_dir());
      dm.settings_->root_dir.assign(tmp_dir);
      TEST_CHECK_TRUE(dm.check_root_dir());
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
        TEST_CHECK_TRUE(success_tx); // one file summary
      }
    }
  }

UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
