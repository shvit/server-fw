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

#include "tftpCommon.h"
#include "tftpBase.h"
#include "tftpIDataMgr.h"
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

  // Properties
  std::atomic<State> stat_;          ///< State machine
  std::atomic_bool   finished_;      ///< Flag: true when session finished
  Addr               my_addr_;       ///< Self server address
  Addr               cl_addr_;       ///< Client address
  int                socket_;        ///< Socket
  size_t             stage_;         ///< Full (!) number of processed block
  //DataMgr            manager_;       ///< Data manager
  uint16_t           error_code_;    ///< First error info - code
  std::string        error_message_; ///< First error info - message
  Options            opt_;           ///< TFTP protocol options
  pIDataMgr          file_man_;

  /** \brief Main constructor
   *
   *  \param [in] new_settings Pointer to exist settings
   */
  Session(pSettings new_settings);

  /** \brief Construct option acknowledge
   *
   *  \param [in,out] buf Buffer for data packet
   */
  void construct_opt_reply(SmBufEx & buf);

  /** \brief Construct error block
   *
   *  Default use: code=0, message="Undefined error"
   *  \param [in,out] buf Buffer for data packet
   */
  void construct_error(SmBufEx & buf);

  /** \brief Construct data block
   *
   *  \param [in,out] buf Buffer for data packet
   */
  void construct_data(SmBufEx & buf);

  /** \brief Construct data block acknowledge
   *
   *  \param [in,out] buf Buffer for data packet
   */
  void construct_ack(SmBufEx & buf);

  /** \brief Get tftp block size
   *
   *  \return Block size value
   */
  auto block_size() const -> uint16_t;

  /** \brief Get current tftp block number
   *
   *  Calculate tftp block number as 16-bit value
   *  \param [in] step Step if need calculate next block number; default 0
   *  \return Block number value
   */
  auto blk_num_local() const -> uint16_t;

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

  /** \brief Try to receive packet if need
   *
   *  No wait - not blocking.
   *  \return True if continue loop, False for break loop
   */
  bool transmit_no_wait(const SmBufEx & buf);

  /** \brief Try to receive packet if exist
   *
   *  No wait - not blocking.
   *  \return True if continue loop, False for break loop
   */
  auto receive_no_wait(SmBufEx & buf) -> TripleResult;

  /** \brief Switch state machine no new state
   *
   *  \param [in] new_state New state
   *  \return True if success, else - false
   */
  bool switch_to(const State & new_state);

  /** \brief Check current window is closed
   *
   *  \return True if closed window, else - false
   */
  bool is_window_close(const size_t & curr_stage) const;

  /** \brief Step back one windowsize
   *
   *  \param [in,out] curr_stage Current full block number
   */
  void step_back_window(size_t & curr_stage);

  /** \brief Option windowsize with type size_t
   *
   *  \return Value
   */
  auto windowsize() const -> size_t;

public:

  /** \brief Default Constructor
   */
  Session();

  /** \brief Constructor from base class
   *
   *  \param [in] base Base class instance
   */
  Session(const Base & base);

  // Deny copy
  Session(const Session & ) = delete;
  Session(      Session & ) = delete;
  auto operator=(const Session & val) = delete;
  auto operator=(Session & val) = delete;

  /** \brief Move operator
   *
   *  \param [in] val Moved value
   *  \return Self reference
   */
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

  /** \brief Initialize sockets and bind port
   *
   *  Need call prepare() before use!
   *  \return True if success, else - false
   */
  bool init();

  /** \brief Main session loop
   *
   *  Need call prepare() and init() before use!
   */
  void run();

  /** \brief Checker finished session
   *
   *  For external use (outside)
   *  \return Value from protected atomic value 'finished_'
   */
  bool is_finished() const;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTP_SESSION_H_ */
