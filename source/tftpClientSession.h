/*
 * tftpClientSession.h
 *
 *  Created on: 22 июн. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPCLIENTSESSION_H_
#define SOURCE_TFTPCLIENTSESSION_H_

#include <fstream>
#include <experimental/filesystem>

#include "tftpCommon.h"
#include "tftpClientSettings.h"
#include "tftpSmBufEx.h"

using namespace std::experimental;

namespace tftp
{

//------------------------------------------------------------------------------

enum class ClientSessionResult: int
{
  fail_init,
  ok,
  fail_run,
};

//------------------------------------------------------------------------------

class ClientSession
{
protected:
  std::ostream *  pstream_;
  ClientSettings  settings_;   ///< Settings for TFTP client
  Addr            local_addr_; ///< Local address
  int             socket_;     ///< Socket
  size_t          stage_;      ///< Full (!) number of processed block
  std::ifstream   file_in_;    ///< Input file stream
  std::ofstream   file_out_;   ///< Output file stream
  size_t          file_size_;  ///< Size for processed data

  /** \brief Local used logger method
   *
   *  Safe use
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  void log(LogLvl lvl, std::string_view msg) const;

  bool init_session();

  auto run_session() -> ClientSessionResult;

public:

  ClientSession();

  ClientSession(std::ostream * stream);

  ClientSession(
      std::ostream * stream,
      int argc,
      char * argv[]);

  bool init(int argc, char * argv[]);

  auto run(int argc, char * argv[]) -> ClientSessionResult;


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

  void close();

};

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPCLIENTSESSION_H_ */
