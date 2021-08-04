/**
 * \file tftpDataMgr.cpp
 * \brief Data manager abstract class module
 *
 *  Base data manager class
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <regex>

#include "tftpDataMgr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

DataMgr::DataMgr():
    request_type_{SrvReq::unknown},
    file_size_{0U},
    set_error_{nullptr}
{
}

// -----------------------------------------------------------------------------

DataMgr::~DataMgr()
{
}

// -----------------------------------------------------------------------------

void DataMgr::set_error_if_first(
    const uint16_t e_cod,
    std::string_view e_msg) const
{
  if(set_error_) set_error_(e_cod, e_msg);
}

// -----------------------------------------------------------------------------

bool DataMgr::match_md5(const std::string & val) const
{
  std::regex regex_md5_pure(constants::regex_template_md5);
  std::smatch sm;

  return std::regex_search(val, sm, regex_md5_pure) &&
         (sm.prefix().str().size() == 0U) &&
         (sm.suffix().str().size() == 0U);
}

//##############################################################################

namespace ext
{

// -----------------------------------------------------------------------------

DataMgr::DataMgr():
    Logger(),
    //dir_data_{Direction::unknown},
    file_size_{0U},
    set_error_{nullptr}
    //log_msg_{nullptr}
{
}

// -----------------------------------------------------------------------------

DataMgr::~DataMgr()
{
}

// -----------------------------------------------------------------------------

void DataMgr::set_error_if_first(
    const uint16_t e_cod,
    std::string_view e_msg) const
{
  if(set_error_) set_error_(e_cod, e_msg);
}

// -----------------------------------------------------------------------------

bool DataMgr::match_md5(std::string_view val) const
{
  std::regex regex_md5_pure(constants::regex_template_md5);
  std::smatch sm;

  std::string str_val{val};

  return std::regex_search(str_val, sm, regex_md5_pure) &&
         (sm.prefix().str().size() == 0U) &&
         (sm.suffix().str().size() == 0U);
}



} // namespace ext

// -----------------------------------------------------------------------------

} // namespace tftp
