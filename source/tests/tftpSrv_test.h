#ifndef SOURCE_TESTS_TFTPSRV_TEST_H_
#define SOURCE_TESTS_TFTPSRV_TEST_H_

#include "test.h"
#include "../tftpSrv.h"
#include "../tftpBase.h"

namespace tftp_server
{
//------------------------------------------------------------------------------

class test_helper: public tftp::Base
{
public:
  static size_t indicator_value_;
  static size_t indicator_couner_;
  static size_t indicator_block_;
private:
  int      socket_;
public:

  test_helper(
      char * loc_addr,
      size_t loc_size,
      char * srv_addr,
      size_t srv_size):
          socket_{0},
          blk_size{512},
          a_local_{},
          a_server_{},
          tx_buf(0xFFFFU, 0),
          rx_buf(0xFFFFU, 0),
          et_buf(0xFFFFU, 0),
          tx_size{0},
          rx_size{0},
          et_size{0}
  {
    a_local_.resize(loc_size);
    if(loc_size) std::copy(loc_addr, loc_addr+loc_size, a_local_.begin());

    if(loc_size>1)
    {
      std::copy(
          loc_addr,
          loc_addr+loc_size,
          a_local_.begin());

      socket_ = socket(
          *((uint16_t *) a_local_.data()),
          SOCK_DGRAM,
          0);

      if(socket_ < 0) return;

      if(int opt=1; setsockopt(
         socket_,
         SOL_SOCKET,
         SO_REUSEADDR,
         (char *) & opt, sizeof(opt)))
      {
        return;
      }

      if(bind(
          socket_,
          (struct sockaddr *) a_local_.data(),
          a_local_.size()))
      {
        close(socket_);
        socket_=0;
      }
    }

    a_server_.resize(srv_size);
    if(srv_size>0)
    {
      std::copy(
          srv_addr,
          srv_addr + srv_size,
          a_server_.begin());
    }
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

  template<typename T>
  void push_tx_cont(T && val)
  {
    tx_size += set_buf_cont_str(tx_buf, tx_size, std::forward<T>(val));
  }

  template<typename T>
  void push_et_cont(T && val)
  {
    et_size += set_buf_cont_str(et_buf, et_size, std::forward<T>(val));
  }

  template<typename T>
  void push_tx_raw(T && val)
  {
    tx_size += set_buf_item_raw(tx_buf, tx_size, val);
  }

  template<typename T>
  void push_et_raw(T && val)
  {
    et_size += set_buf_item_raw(et_buf, et_size, val);
  }

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
      unsigned int rx_client_size = a_server_.size();
      last_size = recvfrom(
          socket_,
          rx_buf.data(),
          rx_buf.size(),
          MSG_DONTWAIT,
          (struct sockaddr *) a_server_.data(),
          & rx_client_size);
      if(last_size > 0)
      {
        rx_size = last_size;
        break;
      }
    }
    return last_size;
  };

  bool make_tx(const size_t step, const size_t file_id) // packet tx makers
  {
    tx_size=0;
    if(step==0) // request
    {
      push_tx_raw(htobe16(2U));
      push_tx_cont(std::string{"tst_srv_file_"}+std::to_string(blk_size)+
                   "_"+std::to_string(file_id+1));
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
      ssize_t left_size = unit_tests::file_sizes[file_id] - (step-1) * blk_size;
      if(left_size > blk_size) left_size = blk_size;
      if(left_size >= 0)
      {
        push_tx_raw(htobe16(3U));
        push_tx_raw(htobe16((uint16_t)(step & 0x000000000000FFFFU)));
        unit_tests::fill_buffer(
            tx_buf.data() + tx_size,
            left_size,
            (step-1) * blk_size,
            file_id);
        tx_size += left_size;
      }
    }

    return tx_size>0;
  };

  // ethalon (for rx) packet makers
  bool make_et(const size_t step, const size_t file_id)
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
    if(ret)
    {
      ret = std::equal(
          rx_buf.cbegin(),
          rx_buf.cbegin() + et_size,
          et_buf.cbegin());
    }
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

//------------------------------------------------------------------------------

} // namespace tftp_server

#endif /* SOURCE_TESTS_TFTPSRV_TEST_H_ */
