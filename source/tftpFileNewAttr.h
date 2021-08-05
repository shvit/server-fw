/**
 * \file tftpFileNewAttr.h
 * \brief FileNewAttr class header
 *
 *  License GPL-3.0
 *
 *  \date 05-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#ifndef SOURCE_TFTPFILENEWATTR_H_
#define SOURCE_TFTPFILENEWATTR_H_

#include "tftpCommon.h"

namespace tftp
{

// -----------------------------------------------------------------------------

/** \brief Class for store settings applied to new files
 *
 *  Use constants:
 *  - default_file_chmod_value
 *  - default_file_chmod_mask
 */
class FileNewAttr
{
protected:

  std::string own_user_; ///< Owner user for created files

  std::string own_grp_;  ///< Owner group for created files

  int         mode_;     ///< File mode for created files

public:

  /** \brief Default constructor
   *
   */
  FileNewAttr();

  /** \brief Full set constructor
   *
   *  For default mode use constants::default_file_chmod_value
   *  \param [in] new_user New owner user
   *  \param [in] new_grp New owner group
   *  \param [in] new_mode New file mode
   */
  FileNewAttr(
      std::string_view new_user,
      std::string_view new_grp,
      int new_mode);

  /** \brief Getter for file owner user
   *
   *  \return Owner user string
   */
  auto own_user() const -> const std::string &;

  /** \brief Getter for file owner group
   *
   *  \return Owner group string
   */
  auto own_grp() const -> const std::string &;

  /** \brief Getter for file mode
   *
   *  \return File mode value
   */
  auto mode() const -> const int &;

  /** \brief Setter for new owner user
   *
   *  \param [in] new_val New value
   */
  void set_own_user(std::string_view new_val);

  /** \brief Setter for new owner group
   *
   *  \param [in] new_val New value
   */
  void set_own_grp(std::string_view new_val);

  /** \brief Setter for new file mode
   *
   *  Use mask from constants::default_file_chmod_mask
   *  \param [in] new_val New mode
   */
  void set_mode(int new_val);

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPFILENEWATTR_H_ */
