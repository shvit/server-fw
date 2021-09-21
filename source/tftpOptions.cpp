/**
 * \file tftpOptions.cpp
 * \brief TFTP Options class module
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include "tftpCommon.h"
#include "tftpOptions.h"

namespace tftp
{

//------------------------------------------------------------------------------

Options::Options():
    request_type_{SrvReq::unknown},
    filename_{},
    transfer_mode_{TransfMode::unknown},
    blksize_   {false, constants::dflt_blksize},
    timeout_   {false, constants::dflt_timeout},
    tsize_     {false, constants::dflt_tsize},
    windowsize_{false, constants::dflt_windowsize}
{
}

//------------------------------------------------------------------------------

Options::~Options()
{
}

//------------------------------------------------------------------------------

auto Options::request_type() const -> const SrvReq &
{
  return request_type_;
}

//------------------------------------------------------------------------------

auto Options::filename() const -> const std::string &
{
  return filename_;
}

//------------------------------------------------------------------------------

auto Options::transfer_mode() const -> const TransfMode &
{
  return transfer_mode_;
}

//------------------------------------------------------------------------------

auto Options::blksize() const -> const int &
{
  return std::get<1>(blksize_);
}

//------------------------------------------------------------------------------

auto Options::timeout() const -> const int &
{
  return std::get<1>(timeout_);
}

//------------------------------------------------------------------------------

auto Options::tsize() const -> const int &
{
  return std::get<1>(tsize_);
}

//------------------------------------------------------------------------------

auto Options::windowsize() const -> const int &
{
  return std::get<1>(windowsize_);
}

//------------------------------------------------------------------------------

bool Options::was_set_blksize() const
{
  return std::get<0>(blksize_);
}

bool Options::was_set_timeout() const
{
  return std::get<0>(timeout_);
}

bool Options::was_set_tsize() const
{
  return std::get<0>(tsize_);
}

bool Options::was_set_windowsize() const
{
  return std::get<0>(windowsize_);
}

bool Options::was_set_any() const
{
  return was_set_blksize() ||
         was_set_timeout() ||
         was_set_tsize() ||
         was_set_windowsize();
}

//------------------------------------------------------------------------------

#define OPT_L_DBG(MSG) if(log != nullptr) L_DBG(MSG);
#define OPT_L_INF(MSG) if(log != nullptr) L_INF(MSG);
#define OPT_L_WRN(MSG) if(log != nullptr) L_WRN(MSG);
#define OPT_L_ERR(MSG) if(log != nullptr) L_ERR(MSG);

bool Options::buffer_parse(
    const SmBuf & buf,
    const size_t & buf_size,
    fLogMsg log)
{
  operator=(Options{});

  bool ret = buf_size >= sizeof(int16_t);

  // Init request type
  size_t curr_pos=0U;
  if(ret)
  {
    if(auto rq_type = buf.get_be<int16_t>(curr_pos);
       (ret = ret && ((rq_type == (int16_t)SrvReq::read) ||
                      (rq_type == (int16_t)SrvReq::write))))
    {
      request_type_ = (SrvReq) rq_type;
      OPT_L_INF("Recognize request type '"+
            std::string{to_string(request_type_)}+"'");
      curr_pos += 2U;
    }
    else
    {
      OPT_L_WRN("Wrong request type ("+std::to_string(rq_type)+")");
    }
  }

  // Init filename
  if(ret)
  {
    auto tmp_str = buf.get_string(curr_pos, buf_size-curr_pos);
    ret = set_filename(tmp_str, log);
    if(ret)
    {
      OPT_L_INF("Recognize filename '"+filename_+"'");
    }
    curr_pos += tmp_str.size()+1U;
  }

  // Init TFTP transfer mode
  if(ret)
  {
    std::string curr_mod = buf.get_string(curr_pos, buf_size-curr_pos);
    ret = set_transfer_mode(curr_mod, log);
    if(ret)
    {
      OPT_L_INF("Recognize tranfser mode '"+curr_mod+"'");
    }
    curr_pos += curr_mod.size()+1U;
  }

  // Init options
  if(ret)
  {
    while(curr_pos < buf_size)
    {
      std::string str_opt{buf.get_string(
          curr_pos,
          buf_size-curr_pos)};
      do_lower(str_opt);
      curr_pos += str_opt.size()+1U;
      if(curr_pos >= buf_size) break;

      std::string str_val{buf.get_string(
          curr_pos,
          buf_size-curr_pos)};
      curr_pos += str_val.size()+1U;

      bool fl=false;
      if(str_opt == constants::name_blksize) { fl=true; set_blksize(str_val, log); }
      else
      if(str_opt == constants::name_timeout) { fl=true; set_timeout(str_val, log); }
      else
      if(str_opt == constants::name_tsize) { fl=true; set_tsize(str_val, log); }
      else
      if(str_opt == constants::name_windowsize) { fl=true; set_windowsize(str_val, log); }
      else
      {
        OPT_L_WRN("Unknown option '"+str_opt+"'='"+str_val+"'; Ignore!");
      }
      if(fl) OPT_L_INF("Recognize option '"+str_opt+"' value '"+str_val+"'");
    }
  }

  return ret;
}

//------------------------------------------------------------------------------

void Options::apply_oack(
    const Options & new_opt,
    fLogMsg log)
{

#define CHECK_OPT(NAME) \
    {\
      std::string oname = "'" + std::string{constants::name_##NAME} + "'";\
      if(was_set_##NAME() == new_opt.was_set_##NAME())\
      {\
        if(was_set_##NAME())\
        {\
          if(NAME() == new_opt.NAME())\
          {\
            OPT_L_DBG("Ack option "+oname);\
          }\
          else\
          {\
            OPT_L_WRN("Try change ack value for option "+oname+"; Ignore self value!");\
            set_##NAME(new_opt.NAME(), log);\
          }\
        }\
      }\
      else\
      {\
        if(was_set_##NAME())\
        {\
          OPT_L_WRN("Option "+oname+" not confirmed! Ignore self value");\
          reset_##NAME();\
        }\
        else\
        {\
          OPT_L_WRN("Option "+oname+" not required but present! Ignore new value");\
        }\
      }\
    }

  CHECK_OPT(blksize);
  CHECK_OPT(timeout);
  CHECK_OPT(windowsize);
  CHECK_OPT(tsize);


  /*
  { // blksize
    std::string oname = "'" + std::string{constants::name_blksize} + "'";
    if(was_set_blksize() == new_opt.was_set_blksize())
    {
      if(was_set_blksize())
      {
        if(blksize() == new_opt.blksize())
        {
          OPT_L_DBG("Ack option "+oname);
        }
        else
        {
          OPT_L_WRN("Try change ack value for option "+oname+"; Ignore self value!");
          set_blksize(new_opt.blksize(), log);
        }
      }
    }
    else // desync blksize
    {
      if(was_set_blksize())
      {
        OPT_L_WRN("Option "+oname+" not confirmed! Ignore self value");
        reset_blksize();
      }
      else
      {
        OPT_L_WRN("Option "+oname+" not required but present! Ignore new value");
      }
    }
  }

  { // timeout
    std::string oname = "'" + std::string{constants::name_timeout} + "'";
    if(was_set_timeout() == new_opt.was_set_timeout())
    {
      if(was_set_timeout())
      {
        if(timeout() == new_opt.timeout())
        {
          OPT_L_DBG("Ack option "+oname);
        }
        else
        {
          OPT_L_WRN("Try change ack value for option "+oname+"; Ignore self value!");
          set_timeout(new_opt.timeout(), log);
        }
      }
    }
    else // desync timeout
    {
      if(was_set_timeout())
      {
        OPT_L_WRN("Option "+oname+" not confirmed! Ignore self value");
        reset_timeout();
      }
      else
      {
        OPT_L_WRN("Option "+oname+" not required but present! Ignore new value");
      }
    }
  }

  { // windowsize
    std::string oname = "'" + std::string{constants::name_windowsize} + "'";
    if(was_set_windowsize() == new_opt.was_set_windowsize())
    {
      if(was_set_windowsize())
      {
        if(windowsize() == new_opt.windowsize())
        {
          OPT_L_DBG("Ack option "+oname);
        }
        else
        {
          OPT_L_WRN("Try change ack value for option "+oname+"; Ignore self value!");
          set_windowsize(new_opt.windowsize(), log);
        }
      }
    }
    else // desync windowsize
    {
      if(was_set_windowsize())
      {
        OPT_L_WRN("Option "+oname+" not confirmed! Ignore self value");
        reset_windowsize();
      }
      else
      {
        OPT_L_WRN("Option "+oname+" not required but present! Ignore new value");
      }
    }
  }
*/

}

//------------------------------------------------------------------------------

bool Options::buffer_parse_oack(
    const SmBuf & buf,
    const size_t & buf_size,
    fLogMsg log)
{
  operator=(Options{});

  bool ret = buf_size >= sizeof(uint16_t);

  size_t curr_pos=0U;
  if(ret)
  {
    if(auto opcode = buf.get_be<int16_t>(curr_pos);
       (ret = opcode == 6U))
    {
      OPT_L_DBG("Recognize OACK pkt");
      curr_pos += 2U;
    }
    else
    {
      OPT_L_WRN("Wrong pkt opcode  ("+std::to_string(opcode)+")");
    }
  }

  if(ret)
  {
    while(curr_pos < buf_size)
    {
      std::string str_opt{buf.get_string(
          curr_pos,
          buf_size-curr_pos)};
      do_lower(str_opt);
      curr_pos += str_opt.size()+1U;
      if(curr_pos >= buf_size) break;

      std::string str_val{buf.get_string(
          curr_pos,
          buf_size-curr_pos)};
      curr_pos += str_val.size()+1U;

      bool fl=false;
      if(str_opt == constants::name_blksize) { fl=true; set_blksize(str_val, log); }
      else
      if(str_opt == constants::name_timeout) { fl=true; set_timeout(str_val, log); }
      else
      if(str_opt == constants::name_tsize) { fl=true; set_tsize(str_val, log); }
      else
      if(str_opt == constants::name_windowsize) { fl=true; set_windowsize(str_val, log); }
      else
      {
        OPT_L_WRN("Unknown option '"+str_opt+"'='"+str_val+"'; Ignore!");
      }
      if(fl) OPT_L_INF("Recognize option '"+str_opt+"' value '"+str_val+"'");
    }
  }

  return ret;
}

//------------------------------------------------------------------------------

bool Options::set_request_type(SrvReq new_req)
{
  request_type_ = new_req;
  return (request_type_ != SrvReq::unknown);
}

//------------------------------------------------------------------------------

bool Options::set_filename(const std::string & val, fLogMsg log)
{
  filename_ = val;

  if(filename_.size() == 0U)
  {
    OPT_L_WRN("Wrong filename (empty!)");
  }

  return filename_.size() > 0U;
}

//------------------------------------------------------------------------------

bool Options::set_blksize(
    const int & val,
    fLogMsg log)
{
  if(val < 1)
  {
    OPT_L_WRN("Wrong value too small ("+std::to_string(val)+"); Ignore!");
    return false;
  }

  if(val > 65500)
  {
    OPT_L_WRN("Wrong value too large ("+std::to_string(val)+"); Ignore!");
    return false;
  }

  blksize_ = {true, val};
  return true;
}

//------------------------------------------------------------------------------

bool Options::set_blksize(
    const std::string & val,
    fLogMsg log)
{
  if(!is_digit_str(val))
  {
    OPT_L_WRN("Wrong value '"+val+"'; Ignore!");
    return false;
  }

  int tmp_int;
  try
  {
    tmp_int = std::stoi(val);
  }
  catch (...)
  {
    OPT_L_WRN("Converting error value '"+val+"'; Ignore!");
    return false;
  }

  return set_blksize(tmp_int, log);
}

//------------------------------------------------------------------------------

bool Options::set_timeout(
    const int & val,
    fLogMsg log)
{

  if(val < 1)
  {
    OPT_L_WRN("Wrong value too small ("+std::to_string(val)+"); Ignore!");
    return false;
  }

  if(val > 3600)
  {
    OPT_L_WRN("Wrong value too large ("+std::to_string(val)+"); Ignore!");
    return false;
  }

  timeout_ = {true, val};
  return true;
}

bool Options::set_timeout(
    const std::string & val,
    fLogMsg log)
{
  if(!is_digit_str(val))
  {
    OPT_L_WRN("Wrong value '"+val+"'; Ignore!");
    return false;
  }

  int tmp_int;
  try
  {
    tmp_int = std::stoi(val);
  }
  catch (...)
  {
    OPT_L_WRN("Converting error value '"+val+"'; Ignore!");
    return false;
  }

  return set_timeout(tmp_int, log);
}

//------------------------------------------------------------------------------

bool Options::set_windowsize(
    const int & val,
    fLogMsg log)
{
  if(val < 1)
  {
    OPT_L_WRN("Wrong value too small ("+std::to_string(val)+"); Ignore!");
    return false;
  }

  windowsize_ = {true, val};
  return true;
}

bool Options::set_windowsize(
    const std::string & val,
    fLogMsg log)
{
  if(!is_digit_str(val))
  {
    OPT_L_WRN("Wrong value '"+val+"'; Ignore!");
    return false;
  }

  int tmp_int;
  try
  {
    tmp_int = std::stoi(val);
  }
  catch (...)
  {
    OPT_L_WRN("Converting error value '"+val+"'; Ignore!");
    return false;
  }

  return set_windowsize(tmp_int, log);
}

//------------------------------------------------------------------------------

bool Options::set_tsize(
    const int & val,
    fLogMsg log)
{
  if(val < 0)
  {
    OPT_L_WRN("Wrong value too small ("+std::to_string(val)+"); Ignore!");
    return false;
  }

  tsize_ = {true, val};
  return true;
}

bool Options::set_tsize(
    const std::string & val,
    fLogMsg log)
{
  if(!is_digit_str(val))
  {
    OPT_L_WRN("Wrong value '"+val+"'; Ignore!");
    return false;
  }

  int tmp_int;
  try
  {
    tmp_int = std::stoi(val);
  }
  catch (...)
  {
    OPT_L_WRN("Converting error value '"+val+"'; Ignore!");
    return false;
  }

  return set_tsize(tmp_int, log);
}

//------------------------------------------------------------------------------

bool Options::set_transfer_mode(
    const std::string & val,
    fLogMsg log)
{
  bool ret = false;

  if(val.size() > 0U)
  {
    std::string curr_mod{val};
    do_lower(curr_mod);

    size_t fl = 0U;
    if(curr_mod == "netascii") { ++fl; transfer_mode_ = TransfMode::netascii; }
    else
    if(curr_mod == "octet") { ++fl; transfer_mode_ = TransfMode::octet; }
    else
    if(curr_mod == "binary") { ++fl; transfer_mode_ = TransfMode::binary; }
    else
    if(curr_mod == "mail") { ++fl; transfer_mode_ = TransfMode::mail; }

    if(!(ret = fl))
    {
      OPT_L_WRN("Wrong value '"+curr_mod+"'; Ignore!");
    }
  }
  else
  {
    OPT_L_WRN("Wrong value (empty!); Ignore!");
  }

  return ret;
}

//------------------------------------------------------------------------------

void Options::reset_blksize()
{
  blksize_ = {false, constants::dflt_blksize};
}

void Options::reset_timeout()
{
  blksize_ = {false, constants::dflt_timeout};
}

void Options::reset_windowsize()
{
  blksize_ = {false, constants::dflt_windowsize};
}

void Options::reset_tsize()
{
  blksize_ = {false, constants::dflt_tsize};
}

void Options::reset_all()
{
  reset_blksize();
  reset_timeout();
  reset_windowsize();
  reset_tsize();
}


//------------------------------------------------------------------------------

#undef OPT_L_INF
#undef OPT_L_WRN

} // namespace tftp

