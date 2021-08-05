/**
 * \file tftpLogger.h
 * \brief Logger base class
 *
 *  License GPL-3.0
 *
 *  \date 03-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPLOGGER_H_
#define SOURCE_TFTPLOGGER_H_

#include "tftpCommon.h"

namespace tftp
{

//------------------------------------------------------------------------------

/** \brief Class for loggung use by callback function
 *
 *  Need add callback before use
 */
class Logger
{
protected:

  fLogMsg log_; ///< Callback logging to top level

public:

  /** \brief Default constructor
   */
  Logger();

  /** \brief Constructor from function
   *
   *  \param [in] new_cd Callback function
   */
  Logger(fLogMsg new_cb);

  /** \brief Copy constructor
   *
   *  \param [in] src Source instance
   */
  Logger(const Logger & src);

  /** \brief Destructor
   */
  virtual ~Logger();

  /** \brief Copy operator
   *
   *  \param [in] src Source instance
   *  \return Self reference
   */
  auto operator=(const Logger & src) -> Logger &;

  /** \brief Setter for callback function
   *
   *  \param [in] new_cd Callback function
   *  \return Self reference
   */
  auto operator=(fLogMsg new_cb) -> Logger &;

  /** \brief Main logger function
   *
   *  Only call back if was set, else do nothing
   *  \param [in] lvl Level of message
   *  \param [in] msg Text message
   */
  void log(LogLvl lvl, std::string_view msg) const;

};

//------------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPLOGGER_H_ */
