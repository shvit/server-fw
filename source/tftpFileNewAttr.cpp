/**
 * \file tftpFileNewAttr.cpp
 * \brief FileNewAttr class module
 *
 *  License GPL-3.0
 *
 *  \date 05-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include "tftpFileNewAttr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

FileNewAttr::FileNewAttr():
    FileNewAttr(
        "",
        "",
        constants::default_file_chmod_value)
{
}

// -----------------------------------------------------------------------------

FileNewAttr::FileNewAttr(
    std::string_view new_user,
    std::string_view new_grp,
    int new_mode):
      own_user_{new_user},
      own_grp_{new_grp},
      mode_{new_mode}
{
}

// -----------------------------------------------------------------------------

auto FileNewAttr::own_user() const -> const std::string &
{
  return own_user_;
}

// -----------------------------------------------------------------------------

auto FileNewAttr::own_grp() const -> const std::string &
{
  return own_grp_;
}

// -----------------------------------------------------------------------------

auto FileNewAttr::mode() const -> const int &
{
  return mode_;
}

// -----------------------------------------------------------------------------

void FileNewAttr::set_own_user(std::string_view new_val)
{
  own_user_.assign(new_val);
}

// -----------------------------------------------------------------------------

void FileNewAttr::set_own_grp(std::string_view new_val)
{
  own_grp_.assign(new_val);
}

// -----------------------------------------------------------------------------

void FileNewAttr::set_mode(int new_val)
{
  mode_ = new_val & constants::default_file_chmod_mask;
}

// -----------------------------------------------------------------------------

} // namespace tftp

