/*
 * tftpIDataMgr.cpp
 *
 *  Created on: 24 мая 2021 г.
 *      Author: svv
 */

#include <regex>

#include "tftpIDataMgr.h"

namespace tftp
{

// -----------------------------------------------------------------------------

IDataMgr::IDataMgr():
    request_type_{SrvReq::unknown},
    file_size_{0U},
    set_error_{nullptr}
{
}

// -----------------------------------------------------------------------------

IDataMgr::~IDataMgr()
{
}

// -----------------------------------------------------------------------------

void IDataMgr::set_error_if_first(
    const uint16_t e_cod,
    std::string_view e_msg) const
{
  if(set_error_) set_error_(e_cod, e_msg);
}

// -----------------------------------------------------------------------------

bool IDataMgr::match_md5(const std::string & val) const
{
  std::regex regex_md5_pure(constants::regex_template_md5);
  std::smatch sm;

  return std::regex_search(val, sm, regex_md5_pure) &&
         (sm.prefix().str().size() == 0U) &&
         (sm.suffix().str().size() == 0U);
}

// -----------------------------------------------------------------------------

} // namespace tftp
