#include <netinet/in.h> // sockaddr

#include "tftpSrv_test.h"

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(Srv)

// Init global counters for dots indicator show

/// block counter dots printed
size_t test_helper::indicator_couner_ = 0;

/// rate dots print (every N bytes)
size_t test_helper::indicator_block_ = 10*1000*1000;

/// processed bytes counter
size_t test_helper::indicator_value_ = 0;

//------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(Srv, "Server main check")
  // 0 prepare
  std::cout << "long time checks [" << std::flush;
  START_ITER("prepare - create temporary directory");
  TEST_CHECK_TRUE(temp_directory_create());

  // 1 Init

  START_ITER("Init - start server");

  const struct sockaddr_in srv_addr // server listening address
  {
    AF_INET,
    htobe16(5151U),
    {0x0100007FU}, // 127.0.0.1
    {0}
  };

  const tftp::Buf listen_addr{((char *) & srv_addr),
                        ((char *) & srv_addr) + sizeof(srv_addr)};

  std::string addr_str = tftp::sockaddr_to_str(
      listen_addr.cbegin(),
      listen_addr.cend());

  const char * tst_arg[]={ "./server_fw",
                           "--syslog", "0",
                           "--ip", addr_str.c_str(),
                           "--root-dir", tmp_dir.c_str() };

  tftp::Srv srv1;
  TEST_CHECK_TRUE(srv1.load_options(
      sizeof(tst_arg)/sizeof(tst_arg[0]),
      const_cast<char **>(tst_arg)));

  TEST_CHECK_TRUE(srv1.init());
  std::thread th_srv1 = std::thread(& tftp::Srv::main_loop, & srv1);
  usleep(500000);

  // 2 Work
  for(auto & item_blksize :
      std::vector<uint16_t>{100,512,1024,1400,65000}) // iteration over blksizes
  {
    for(size_t file_id=0;
        file_id < file_sizes.size();
        ++file_id) // iteration over files (sizes)
    {
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port   = htobe16(60000U+file_id);
      addr.sin_addr.s_addr   = 0x00000000U;

      test_helper hlpr(
          (char *) & addr,
          sizeof(addr),
          (char *) & srv_addr,
          sizeof(srv_addr));

      hlpr.blk_size = item_blksize;

      for(size_t stage = 0;
          ((ssize_t)stage - 1) * (ssize_t)hlpr.blk_size <=
              (ssize_t)file_sizes[file_id];
          ++stage)
      {
        TEST_CHECK_TRUE(hlpr.full_tx(stage, file_id));      // tx pkt
        TEST_CHECK_TRUE(hlpr.full_rx(stage, file_id, 8));   // rx pkt (reply)
      }
      usleep(200000);
    }
  }

  // 3 Finish
  START_ITER("finish");
  srv1.stop();
  usleep(200000);
  th_srv1.join();
  usleep(200000);

  // end
  std::cout << "]" << std::endl;
UNIT_TEST_CASE_END

//------------------------------------------------------------------------------

UNIT_TEST_SUITE_END

