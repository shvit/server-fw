/*
 * tftpClientSession.h
 *
 *  Created on: 22 июн. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPCLIENTSESSION_H_
#define SOURCE_TFTPCLIENTSESSION_H_

#include <atomic>
#include <fstream>
#include <experimental/filesystem>

#include "tftpCommon.h"
#include "tftpClientSettings.h"
#include "tftpSmBufEx.h"

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
class ClientSession
{
protected:
  std::ostream *   pstream_;
  ClientSettings   settings_;   ///< Settings for TFTP client
  Addr             local_addr_; ///< Local address
  int              socket_;     ///< Socket
  size_t           stage_;      ///< Full (!) number of processed block
  std::ifstream    file_in_;    ///< Input file stream
  std::ofstream    file_out_;   ///< Output file stream
  size_t           file_size_;  ///< Size for processed data

  std::atomic_bool need_break_; ///< Flag for stop session
  std::atomic_bool stopped_;    ///< Flag for stop session

  /** \brief Local used logger method
   *
   *  Safe use
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  void log(LogLvl lvl, std::string_view msg) const;

  /** \brief Init session
   *
   *  Doing:
   *  - check file
   *  - open file
   *  - open socket
   *  - check some arguments
   *  \return True is initialize success, else - false
   */
  bool init_session();

  /** \brief Run client TFTP session
   *
   *  Need already inited
   *  \return Value of enum type ClientSessionResult
   */
  auto run_session() -> ClientSessionResult;

public:

  /** \brief Default constructor
   */
  ClientSession();

  /** \brief Constructor with output info stream
   *
   *  \param [out] stream Pointer to output stream
   */
  ClientSession(std::ostream * stream);

  /** \brief Main constructor with parse cmd arguments
   *
   *  Internal execute init()
   *  \param [out] stream Pointer to output stream
   *  \param [in] argc Count of arguments (array items)
   *  \param [in] argv Array with arguments
   */
  ClientSession(
      std::ostream * stream,
      int argc,
      char * argv[]);

  /** \brief Initialize session
   *
   *  Internal execute init_session()
   *  \param [in] argc Count of arguments (array items)
   *  \param [in] argv Array with arguments
   *  \return True is initialize success, else - false
   */
  bool init(int argc, char * argv[]);

  /** \brief Run client TFTP session with init
   *
   *  Internal execute init() and run_session()
   *  \param [in] argc Count of arguments (array items)
   *  \param [in] argv Array with arguments
   *  \return Value of enum type ClientSessionResult
   */
  auto run(int argc, char * argv[]) -> ClientSessionResult;

  /** \brief Check opened any file
   *
   *  \return True if in/out file stream opened
   */
  bool active() const;

  /** \brief Pull data from network (receive)
   *
   *  Overrided virtual method for file streams
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position received block
   *  \return 0 on success, -1 on error
   */
  auto write(
      SmBufEx::const_iterator buf_begin,
      SmBufEx::const_iterator buf_end,
      const size_t & position) -> ssize_t;

  /** \brief Push data to network (transmit)
   *
   *  Overrided virtual method for file streams
   *  \param [in] buf_begin Buffer begin iterator
   *  \param [in] buf_end Buffer end iterator
   *  \param [in] position Position transmitted block
   *  \return Processed size, -1 on error
   */
  auto read(
      SmBufEx::iterator buf_begin,
      SmBufEx::iterator buf_end,
      const size_t & position) -> ssize_t;

  /** \brief Close all opened files and close opened socket
   */
  void close();

  /** \brief Close all opened files and close opened socket
   *
   *  Local file will delete if stream was opened
   */
  void cancel();

  void need_break();

};

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPCLIENTSESSION_H_ */
