#include <iostream>
//#include <ostream>
#include <sys/socket.h>
#include <netinet/in.h> // sockaddr
#include <type_traits>


#include "../tftpCommon.h"
#include "../tftpSrv.h"
#include "test.h"     

using namespace unit_tests;

UNIT_TEST_SUITE_BEGIN(tftp_server)

class test_server: public tftp::Srv
{
public:
};

class test_helper: public tftp::Base
{
public:
  static size_t indicator_value_;
  static size_t indicator_couner_;
  static size_t indicator_block_;
private:
  int      socket_;
/*
  void dump(const uint16_t id) // id: 1-tx, 3-et
  {
    assert((id==1) || (id==3)); // || (id==2)
    std::cout << "#" << id << " dump[" << (id==1 ? tx_size : (id==2 ? rx_size : et_size)) << "]=";
    for(size_t iter=0; iter<(id==1 ? tx_size : (id==2 ? rx_size : et_size)); ++iter)
    {
      if(iter) std::cout << " ";
      std::cout << std::hex << std::setw(2) << std::setfill('0');
      std::cout << (((int16_t) (id==1 ? tx_buf[iter] : (id==2 ? rx_buf[iter] : et_buf[iter]))) & 0x00FF);
    }
    std::cout << std::endl << std::dec;
  };
*/
public:
  test_helper(char * loc_addr, size_t loc_size, char * srv_addr, size_t srv_size)
    : socket_{0},
      blk_size{512}, a_local_{}, a_server_{},
      tx_buf(0xFFFFU, 0), rx_buf(0xFFFFU, 0), et_buf(0xFFFFU, 0),
      tx_size{0}, rx_size{0}, et_size{0}
  {
    a_local_.resize(loc_size);
    if(loc_size) std::copy(loc_addr, loc_addr+loc_size, a_local_.begin());

    if(loc_size>1)
    {
      std::copy(loc_addr, loc_addr+loc_size, a_local_.begin());

      socket_=socket(*((uint16_t *) a_local_.data()), SOCK_DGRAM, 0);
      if(socket_ < 0) return;

      if(int opt=1; setsockopt(socket_,  SOL_SOCKET, SO_REUSEADDR, (char *) & opt, sizeof(opt))) return;

      if(bind(socket_, (struct sockaddr *) a_local_.data(), a_local_.size()))
      {
        close(socket_);
        socket_=0;
      }
    }

    a_server_.resize(srv_size);
    if(srv_size>0) std::copy(srv_addr, srv_addr+srv_size, a_server_.begin());
  };

  ~test_helper() { if(socket_ > 0) close(socket_); };

  uint16_t             blk_size;
  tftp::Buf            a_local_;   // addr local (source)
  tftp::Buf            a_server_;  // addr server for requests only
  tftp::Buf            tx_buf;     // tx data buffer
  tftp::Buf            rx_buf;     // rx data buffer
  tftp::Buf            et_buf;     // ethalon data buffer
  tftp::Buf::size_type tx_size;    // tx data size
  tftp::Buf::size_type rx_size;    // rx data size
  tftp::Buf::size_type et_size;    // ethalon data size

  template<typename T> void push_tx_cont(T && val) { tx_size += set_buf_cont_str(tx_buf, tx_size, std::forward<T>(val)); }
  template<typename T> void push_et_cont(T && val) { et_size += set_buf_cont_str(et_buf, et_size, std::forward<T>(val)); }

  template<typename T> void push_tx_raw(T && val) { tx_size += set_buf_item_raw(tx_buf, tx_size, val); }
  template<typename T> void push_et_raw(T && val) { et_size += set_buf_item_raw(et_buf, et_size, val); }

  bool tx()
  {
    bool ret = true;
    if(tx_size > 0)
    {
      ssize_t ok_size = sendto(socket_,
                               tx_buf.data(),
                               tx_size,
                               0,
                               (struct sockaddr *) a_server_.data(),
                               a_server_.size());
      ret = (static_cast<size_t>(ok_size) == tx_size);
    }
    return ret;
  };

  ssize_t rx(size_t timeout)
  {
    rx_size = 0;
    ssize_t last_size=0;

    time_t start = time(nullptr);
    while((time(nullptr) - start) < (ssize_t)timeout)
    {
      socklen_t  rx_client_size = a_server_.size();
      last_size = recvfrom(socket_, rx_buf.data(), rx_buf.size(), MSG_DONTWAIT, (struct sockaddr *) a_server_.data(), & rx_client_size);
      if(last_size > 0)
      {
        rx_size = last_size;
        break;
      }
    }
    return last_size;
  };

  //void dump_tx() { dump(1); };
  //void dump_rx() { dump(2); };
  //void dump_et() { dump(3); };

  bool make_tx(const size_t step, const size_t file_id) // packet tx makers
  {
    tx_size=0;
    if(step==0) // request
    {
      push_tx_raw(htobe16(2U));
      push_tx_cont(std::string{"tst_srv_file_"}+std::to_string(blk_size)+"_"+std::to_string(file_id+1));
      push_tx_raw('\0');
      push_tx_cont(std::string{"octet"});
      push_tx_raw('\0');
      if(blk_size != 512) // if not default block size
      {
        push_tx_cont(std::string{"blksize"});
        push_tx_raw('\0');
        push_tx_cont(std::to_string(blk_size));
        push_tx_raw('\0');
      }
    }
    else
    {
      ssize_t left_size = file_sizes[file_id] - (step-1) * blk_size;
      if(left_size > blk_size) left_size = blk_size;
      if(left_size >= 0)
      {
        push_tx_raw(htobe16(3U));
        push_tx_raw(htobe16((uint16_t)(step & 0x000000000000FFFFU)));
        fill_buffer(tx_buf.data()+tx_size, left_size, (step-1) * blk_size, file_id);
        tx_size += left_size;
      }
    }

    return tx_size>0;
  };

  bool make_et(const size_t step, const size_t file_id)// ethalon (for rx) packet makers
  {
    et_size=0;

    if(step == 0)
    {
      if((blk_size != 512)) // if not default block size
      {
        push_et_raw(htobe16(6U));
        if(blk_size != 512)
        {
          push_et_cont(std::string{"blksize"});
          push_et_raw('\0');
          push_et_cont(std::to_string(blk_size));
          push_et_raw('\0');
        }
      }
      else // no confirm options
      {
        push_et_raw(htobe16(4U));
        push_et_raw(htobe16(0U));
      }
    }
    else
    {
      push_et_raw(htobe16(4U));
      push_et_raw(htobe16((uint16_t)(step & 0x000000000000FFFFU)));
    }

    return et_size > 0;
  };

  bool full_tx(const size_t step, const size_t iteration)
  {
    push_counter(tx_size);
    return make_tx(step, iteration) && tx();
  };

  bool full_rx(const size_t step, const size_t iteration, size_t timeout)
  {
    bool ret=make_et(step, iteration);
    //dump_et();
    ret = (rx(timeout) == (ssize_t)et_size);
    //dump_rx();
    if(ret) ret = std::equal(rx_buf.cbegin(), rx_buf.cbegin()+et_size, et_buf.cbegin());
    push_counter(et_size);
    return  ret;
  };

  void push_counter(size_t val)
  {
    indicator_value_ += val;
    while(indicator_value_ > (indicator_couner_ * indicator_block_))
    {
      ++indicator_couner_;
      std::cout << "." << std::flush;
    }

  };
};

// global counters for dots indicator show
size_t test_helper::indicator_couner_ = 0;          // block counter dots printed
size_t test_helper::indicator_block_ = 10*1000*1000; // rate dots print (every N bytes)
size_t test_helper::indicator_value_ = 0;           // processed bytes counter

//[[maybe_unused]]
//auto logger_tst = [](const int level, std::string_view message)
//  {
      //std::cout << "{" << tftp::to_string_lvl(level) << "} " << message << std::endl;
  //};

//=================================================================================================================================

UNIT_TEST_CASE_BEGIN(server, "server main check")
  // 0 prepare
  std::cout << "long time checks [" << std::flush;
  START_ITER("prepare - create temporary directory");
  TEST_CHECK_TRUE(temp_directory_create());

  // 1 Init

  START_ITER("Init - start server");

  const struct sockaddr_in srv_addr{ // server listening address
    AF_INET,
    htobe16(5151U),
    {0x0100007FU}, // 127.0.0.1
    {0}
  };

  const std::vector<char> listen_addr{((char *) & srv_addr),
                                      ((char *) & srv_addr) + sizeof(srv_addr)};

  std::string addr_str = tftp::sockaddr_to_str(listen_addr.cbegin(), listen_addr.cend());

  const char * tst_arg[]={ "./server_fw",
                           "--syslog", "0",
                           "--ip", addr_str.c_str(),
                           "--root-dir", tmp_dir.c_str() };

  test_server srv1;
  //srv1.set_logger(logger_tst);
  TEST_CHECK_TRUE(srv1.load_options(sizeof(tst_arg)/sizeof(tst_arg[0]), const_cast<char **>(tst_arg)));
  TEST_CHECK_TRUE(srv1.init());
  std::thread th_srv1 = std::thread(& test_server::main_loop, & srv1);
  usleep(500000);

  // 2 Work
  for(auto & item_blksize : std::vector<uint16_t>{100,512,1024,1400,65000}) // iteration over blksizes
  {
    for(size_t file_id=0; file_id < file_sizes.size(); ++file_id) // iteration over files (sizes)
    {
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port   = htobe16(60000U+file_id);
      addr.sin_addr.s_addr   = 0x00000000U;

      test_helper hlpr((char *) & addr, sizeof(addr), (char *) & srv_addr, sizeof(srv_addr));
      hlpr.blk_size = item_blksize;

      for(size_t stage = 0; (static_cast<ssize_t>(stage)-1) * static_cast<ssize_t>(hlpr.blk_size) <= static_cast<ssize_t>(file_sizes[file_id]); ++stage)
      {
        TEST_CHECK_TRUE(hlpr.full_tx(stage, file_id));      // transmit packet
        TEST_CHECK_TRUE(hlpr.full_rx(stage, file_id, 8));   // receive packet (server reply)
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

//---------------------------------------------------------------------------------------------------------------------------------

UNIT_TEST_SUITE_END

