/**
 * \file tftpSrvSettings.cpp
 * \brief SrvSettings class module
 *
 *  License GPL-3.0
 *
 *  \date 04-aug-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include "tftpSrvSettings.h"

namespace tftp
{

// -----------------------------------------------------------------------------

SrvSettings::SrvSettings(const pSrvSettingsStor & sett):
    settings_{sett}
{
}

// -----------------------------------------------------------------------------

SrvSettings::SrvSettings():
    SrvSettings(SrvSettingsStor::create())
{
}

// -----------------------------------------------------------------------------

SrvSettings::SrvSettings(const SrvSettings & src):
    SrvSettings(src.settings_)
{
}

// -----------------------------------------------------------------------------

auto SrvSettings::operator=(const SrvSettings & src) -> SrvSettings &
{
  settings_ = src.settings_;

  return *this;
}

// -----------------------------------------------------------------------------

auto SrvSettings::begin_shared() const -> std::shared_lock<std::shared_mutex>
{
  if(!settings_)
  {
    throw std::runtime_error("Empty storage (server settings)");
  }

  return settings_->begin_shared();
}

// -----------------------------------------------------------------------------

auto SrvSettings::begin_unique() const -> std::unique_lock<std::shared_mutex>
{
  if(!settings_)
  {
    throw std::runtime_error("Empty storage (server settings)");
  }

  return settings_->begin_unique();
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_root_dir() const -> std::string
{
  auto lk = begin_shared(); // read lock

  if(settings_->root_dir.size())
  {
    bool fin_slash =
        settings_->root_dir.size() &&
        (*--settings_->root_dir.end() == '/');

    return settings_->root_dir + (fin_slash ? "" : "/");
  }
  else
  {
    return "";
  }
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_lib_dir() const -> std::string
{
  auto lk = begin_shared(); // read lock

  bool fin_slash =
      settings_->lib_dir.size() &&
      (*--settings_->lib_dir.end() == '/');

  return settings_->lib_dir + (fin_slash ? "" : "/");
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_lib_name_fb() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->lib_name};
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_retransmit_count() const -> uint16_t
{
  auto lk = begin_shared(); // read lock

  return settings_->retransmit_count_;
}

// -----------------------------------------------------------------------------

bool SrvSettings::get_is_daemon() const
{
  auto lk = begin_shared(); // read lock

  return settings_->is_daemon;
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_serach_dir() const -> VecStr
{

  auto lk = begin_shared(); // read lock

  VecStr ret;
  for(const auto & one_dir: settings_->search_dirs)
  {
    ret.emplace_back(one_dir);
  }

  return ret;
}

// -----------------------------------------------------------------------------

int SrvSettings::get_file_chmod() const
{
  auto lk = begin_shared(); // read lock

  return settings_->file_new_attr.mode();
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_file_chown_user() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->file_new_attr.own_user()};
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_file_chown_grp() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->file_new_attr.own_grp()};
}

// -----------------------------------------------------------------------------

auto SrvSettings::get_verb() const -> LogLvl
{
  auto lk = begin_shared(); // read lock

  return ((settings_->verb >= 0) &&
          (settings_->verb <= 7)) ? (LogLvl) settings_->verb : LogLvl::debug;
}

// -----------------------------------------------------------------------------

auto SrvSettings::load_options(
    fLogMsg cb_logger,
    ArgParser ap) -> TripleResult
{
  auto lk = begin_unique(); // write lock

  return settings_->load_options(cb_logger, ap);
}

// -----------------------------------------------------------------------------

} // namespace tftp
