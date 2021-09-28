/*
 * tftpClientSession.h
 *
 *  Created on: 22 июн. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPCLIENTSESSION_H_
#define SOURCE_TFTPCLIENTSESSION_H_

#include <atomic>
#include <experimental/filesystem>

#include "tftpCommon.h"
#include "tftpClientSettings.h"
#include "tftpSmBufEx.h"
#include "tftpDataMgr.h"

using namespace std::experimental;

namespace tftp
{

//------------------------------------------------------------------------------

/** \brief Result of ClientSession::run()
 */
enum class ClientSessionResult: int
{
  fail_init,
  ok,
  fail_run,
};

//------------------------------------------------------------------------------

/** \brief Class of TFTP client session
 *
 *  Simple use (without logging):
 *  auto result = ClientSession().run(argc, argv);
 */
class ClientSession: public Logger
{
protected:
  aState           stat_;       ///< State machine
  pClientSettings  settings_;   ///< Settings for TFTP client
  Addr             local_addr_; ///< Local address
  int              socket_;     ///< Socket
  size_t           stage_;      ///< Full (!) number of processed block
  size_t           file_size_;  ///< Size for processed data
  uint16_t         error_code_;    ///< First error info - code
  std::string      error_message_; ///< First error info - message

  std::atomic_bool need_break_; ///< Flag for stop session
  std::atomic_bool stopped_;    ///< Flag for stop session

  pDataMgr         file_man_;   ///< Data manager
  bool             srv_session_set_;

  /** \brief Switch state machine no new state
   *
   *  \param [in] new_state New state
   *  \return True if success, else - false
   */
  bool switch_to(const State & new_state);

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
  bool was_error() const;

  /** \brief Get current tftp block number
   *
   *  Calculate tftp block number as 16-bit value
   *  \param [in] step Step if need calculate next block number; default 0
   *  \return Block number value
   */
  auto blk_num_local() const -> uint16_t;

  /** \brief Get tftp block size
   *
   *  \return Block size value
   */
  auto block_size() const -> uint16_t;

  /** \brief Construct tftp request
   *
   *  \param [in,out] buf Buffer for data packet
   */
  void construct_request(SmBufEx & buf) const;

  /** \brief Construct error block
   *
   *  Anyway construct tftp error packet
   *  Get data from error_code_ and error_message_
   *  If error not was set, use: code=0, message="Undefined error"
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

  bool is_window_close(const size_t & curr_stage) const;

public:

  /** \brief Default constructor
   *
   *  Create empty settings (need init)
   */
  ClientSession();

  /** \brief Constructor fwith exist ClientSettings
   *
   *  \param [out] sett Client settings unique pointer
   */
  ClientSession(pClientSettings && sett, fLogMsg new_cb);

  static auto create(pClientSettings && sett, fLogMsg new_cb) -> pClientSession;

  /** \brief Init session
   *
   *  Doing:
   *  - check file
   *  - open file
   *  - open socket
   *  - check some arguments
   *  \return True is initialize success, else - false
   */
  bool init();

  /** \brief Run client TFTP session with init
   *
   *  Internal execute init() and run_session()
   *  \param [in] argc Count of arguments (array items)
   *  \param [in] argv Array with arguments
   *  \return Value of enum type ClientSessionResult
   */
  auto run() -> ClientSessionResult;

  /** \brief Run client TFTP session
   *
   *  Need already inited
   *  \return Value of enum type ClientSessionResult
   */
  auto run_session() -> ClientSessionResult;

  /** \brief Check opened any file
   *
   *  \return True if in/out file stream opened
   */
  //bool active() const;

  /** \brief Pull data from network (receive)
   *
   *  Overrided virtual method for file streams
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position received block
   *  \return 0 on success, -1 on error
   */
  //auto write(
  //    SmBufEx::const_iterator buf_begin,
  //    SmBufEx::const_iterator buf_end,
  //    const size_t & position) -> ssize_t;

  /** \brief Push data to network (transmit)
   *
   *  Overrided virtual method for file streams
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position transmitted block
   *  \return Processed size, -1 on error
   */
  //auto read(
  //    SmBufEx::iterator buf_begin,
  //    SmBufEx::iterator buf_end,
  //    const size_t & position) -> ssize_t;

  /** \brief Close all opened files and close opened socket
   */
  void close();

  /** \brief Close all opened files and close opened socket
   *
   *  Local file will delete if stream was opened
   */
  void cancel();

  void need_break();

  bool is_finished() const;

  //void set_srv_session(uint16_t new_port);

};

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPCLIENTSESSION_H_ */
