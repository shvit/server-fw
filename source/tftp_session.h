/**
 * \file tftp_session.h
 * \brief TFTP session class header
 *
 *  TFTP session class header
 *
 *  License GPL-3.0
 *
 *  \date   01-dec-2019
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#ifndef SOURCE_TFTP_SESSION_H_
#define SOURCE_TFTP_SESSION_H_

#include <algorithm>
#include <map>
#include <string>
#include <thread>

#include "tftp_common.h"
#include "tftp_data_mgr.h"

namespace tftp
{

// ----------------------------------------------------------------------------------

/**
 * \brief TFTP session class 'tftp::session'
 *
 *  Class for emulate p2p UDP/IP session when request received by server.
 *  For use class:
 *  - Assign global settings.
 *  - Initialize with init(client socket address, recived packet).
 *  - execute run() for one instance or run_thread() for multi-thread
 *
 */

class session: public Base
{
protected:
  srv_req            request_type_;    ///< Server request
  std::string        filename_;        ///< Requested filename
  transfer_mode      transfer_mode_;   ///< Transfer mode
  Buf           client_;          ///< Client socket address buffer
  int                socket_;          ///< Socket
  option_t<uint16_t> opt_blksize_;     ///< Option 'blksize'   : {was writed, value}
  option_t<int>      opt_timeout_;     ///< Option 'timeout'   : {was writed, value}
  option_t<int>      opt_tsize_;       ///< Option 'tsize'     : {was writed, value}
  uint16_t           re_tx_count_;     ///< Retransmitt count
  Buf           sess_buffer_tx_;  ///< Session buffer for TX operations
  Buf           sess_buffer_rx_;  ///< Session buffer for RX operations
  size_t             stage_;           ///< Stage
  size_t             buf_size_tx_;     ///< TX data size
  time_t             oper_time_;       ///< Last remembered action time
  uint16_t           oper_tx_count_;   ///< Transmit try count
  bool               oper_wait_;       ///< Flag r/w state (mode)
  size_t             oper_last_block_; ///< Last (finish) block number
  bool               stop_;            ///< Break loop request (when error, etc.)
  bool               finished_;        ///< confirm (reply) break loop request
  data_mgr           manager_;         ///< Data manager
  uint16_t           error_code_;      ///< First error info - code
  std::string        error_message_;   ///< First error info - message

  /** \brief Get uint16_t from RX buffer with offset
   *
   *  Network byte order value read as host order value
   *  \param [in] offset Index array of uint16_t values
   *  \return Extracted value (host byte order)
   */
  auto get_buf_rx_u16_ntoh(const size_t offset);

  /** \brief Set uint16_t to TX buffer with offset
   *
   *  Host byte order value write as network order value
   *  \param [in] offset Index array of uint16_t values
   *  \param [in] value Value (host byte order) to write buffer
   */
  void set_buf_tx_u16_hton(const size_t offset, const uint16_t value);


  /** \brief Check size for append to buffer and resize if need
   *
   *  \param [in] size_append New size for append
   */
  void check_buffer_tx_size(const size_t size_append);

  /** \brief Append string to buffer
   *
   *  Append zero at the end
   *  If string length is zero do nothing
   *  \param [in] str String value for add
   *  \return Size added data to buffer
   */
  size_t push_buffer_string(std::string_view str);

  /** \brief Construct option acknowledge
   *
   *  Construct tftp packet payload as option acknowledge
   */
  void construct_opt_reply();

  /** \brief Construct error block
   *
   *  Construct tftp packet payload as error block
   *  \param [in] e_code Error code
   *  \param [in] e_msg Error message
   */
  void construct_error(const uint16_t e_code, std::string_view e_msg);

  /** \brief Construct error block
   *
   *  Construct tftp packet payload as error block
   *  Use error settings; default use {0, "Undefined error"}
   */
  void construct_error();

  /** \brief Construct data block
   *
   *  Construct tftp packet payload as data block
   *  Fill buffer and set buf_size_tx_; if can't do it then construct error block
   */
  void construct_data();

  /** \brief Construct data block acknowledge
   *
   *  Construct tftp packet payload as acknowledge
   *  Fill buffer and set buf_size_tx_
   */
  void construct_ack();

  /** \brief Get tftp block size
   *
   *  \return Block size value
   */
  uint16_t block_size() const;

  /** \brief Check timeout passed
   *
   *  \param gandicap Gandicap for time check in seconds
   *  \return True if timeout not out, else (time is out) - false
   */
  bool timeout_pass(const time_t gandicap = 0) const;

  /** \brief Reset time counter
   */
  void timeout_reset();

  /** \brief Get current tftp block number
   *
   *  Calculate tftp block number as 16-bit value
   *  \param [in] step Step if need calculate next block number; default 0
   *  \return Block number value
   */
  uint16_t blk_num_local(const uint16_t step = 0) const;

  /** \brief Open session socket and tune
   */
  bool socket_open();

  /** \brief Close session socket
   */
  void socket_close();

  /** \brief Set error code and message
   *
   *  If values was assigned, then ignore new code and message
   *  (First error has high priority)
   *  \param [in] e_cod Error code
   *  \param [in] e_msg Error message
   */
  void set_error_if_first(const uint16_t e_cod, std::string_view e_msg);

  /** \brief Check was error or not
   *
   *  \return Teue if error code and message assigned, else - false
   */
  bool was_error();

  /** \brief Check current stage is receive
   *
   *  \return True if current stage is receive, else return false
   */
  bool is_stage_receive() const noexcept;

  /** \brief Check current stage is transmit
   *
   *  \return True if current stage is transmit, else return false
   */
  bool is_stage_transmit() const noexcept;

  /** \brief Switch current stage to receive
   */
  void set_stage_receive() noexcept;

  /** \brief Switch current stage to transmit
   */
  void set_stage_transmit() noexcept;

  /** \brief Try to receive packet if need
   *
   *  No wait - not blocking.
   *  \return True if continue loop, False for break loop
   */
  bool transmit_no_wait();

  /** \brief Try to receive packet if exist
   *
   *  No wait - not blocking.
   *  \return True if continue loop, False for break loop
   */
  bool receive_no_wait();

public:
  /** \brief Constructor
   */
  session();

  // Deny copy
  session(const session & ) = delete;
  session(      session & ) = delete;
  session(      session &&) = delete;

  /** \brief Destructor
   */
  virtual ~session();

  /** \brief Session initialize
   *
   *  Initialize session from top level after receive tftp request
   *  \param [in] addr_begin Client socket - begin buffer iterator
   *  \param [in] addr_end Client socket - end buffer iterator
   *  \param [in] buf_begin UDP request data packet - begin buffer iterator
   *  \param [in] buf_end UDP request data packet - end buffer iterator
   *  \return True if initialize success, else - false
   */
  bool init(const Buf::const_iterator addr_begin,
            const Buf::const_iterator addr_end,
            const Buf::const_iterator buf_begin,
            const Buf::const_iterator buf_end);

  /** \brief Main session loop
   */
  void run();

  /** \brief Main session loop as thread executed
   *
   *  \return Instance of std::thread
   */
  std::thread run_thread();

  friend class srv;
};

// ----------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_SESSION_H_ */
