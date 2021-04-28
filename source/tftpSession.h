/**
 * \file tftpSession.h
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

#include <atomic>
#include <thread>
#include <map>
#include <optional>

#include "tftpCommon.h"
#include "tftpDataMgr.h"
#include "tftpSmBuf.h"
#include "tftpOptions.h"
#include "tftpAddr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

/**
 * \brief TFTP session class 'tftp::Session'
 *
 *  Class for emulate p2p UDP/IP session when request received by server.
 *  For use class:
 *  - Assign settings.
 *  - Initialize
 *  - Run main loop run()
 *
 */
class Session: public Base
{
protected:
  Addr         cl_addr_;           ///< Client socket address buffer
  int          socket_;           ///< Socket
  SmBuf        buf_tx_;           ///< Session buffer for TX operations
  SmBuf        buf_rx_;           ///< Session buffer for RX operations
  size_t       stage_;            ///< Stage
  size_t       buf_tx_data_size_; ///< TX data size
  time_t       oper_time_;        ///< Last remembered action time
  uint16_t     oper_tx_count_;    ///< Transmit try count
  bool         oper_wait_;        ///< Flag r/w state (mode)
  size_t       oper_last_block_;  ///< Last (finish) block number
  bool         stop_;             ///< Break loop request (when error, etc.)
  std::atomic_bool finished_;     ///< External use flag: true when session finished
  DataMgr      manager_;          ///< Data manager
  uint16_t     error_code_;       ///< First error info - code
  std::string  error_message_;    ///< First error info - message

  Options      opt_;

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
   *  Fill buffer and set buf_tx_data_size_;
   *  if can't do it then construct error block
   */
  void construct_data();

  /** \brief Construct data block acknowledge
   *
   *  Construct tftp packet payload as acknowledge
   *  Fill buffer and set buf_tx_data_size_
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
  void set_error_if_first(
      const uint16_t e_cod,
      std::string_view e_msg);

  /** \brief Check was error or not
   *
   *  \return Teue if error code and message assigned, else - false
   */
  bool was_error();

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

  template<typename T>
  void push_data(T && value);

public:
  /** \brief Constructor
   */
  Session();

  Session(pSettings & new_settings);

  // Deny copy
  Session(const Session & ) = delete;
  Session(      Session & ) = delete;
  Session(      Session &&) = delete;

  auto operator=(Session && val) -> Session &;

  /** \brief Destructor
   */
  virtual ~Session();

  /** \brief Session initialize
   *
   *  Initialize session from top level after receive tftp request
   *  \param [in] remote_addr Client socket
   *  \param [in] pkt_data Request data packet buffer
   *  \param [in] pkt_data_size Packet buffer size
   *  \return True if initialize success, else - false
   */
  bool prepare(
      const Addr & remote_addr,
      const SmBuf  & pkt_data,
      const size_t & pkt_data_size);

  bool init();

  /** \brief Main session loop
   */
  void run();

  /** \brief Checker finished session
   *
   *   For external use
   *  \return Value from protected atomic value 'finished_'
   */
  bool is_finished() const;

};

// -----------------------------------------------------------------------------

template<typename T>
void Session::push_data(T && value)
{
  if constexpr (std::is_integral_v<T>)
  {
    if((buf_tx_data_size_ + sizeof(T)) <= buf_tx_.size())
    {
      buf_tx_data_size_ += buf_tx_.set_hton(buf_tx_data_size_, value);
    }
  }
  else
  if constexpr (std::is_constructible_v<std::string, T>)
  {
    std::string tmp_str{std::forward<T>(value)};

    if((buf_tx_data_size_ + tmp_str.size()) <= buf_tx_.size())
    {
      buf_tx_data_size_ += buf_tx_.set_string(buf_tx_data_size_, tmp_str, true);
    }
  }
  else // Never do it!
  {
    assert(false); // Wrong use push_data() - Does't support pushed type
  }
}

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_SESSION_H_ */
