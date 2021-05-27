/**
 * \file tftpBase.h
 * \brief Base TFTP server class module
 *
 *  Base class for TFTP server
 *
 *  License GPL-3.0
 *
 *  \date 29-may-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <syslog.h>

#include "tftpBase.h"

namespace tftp
{

// -----------------------------------------------------------------------------

Base::Base():
    Base(nullptr)
{
}

// -----------------------------------------------------------------------------

Base::Base(const pSettings & sett):
    settings_{sett}
{
  if(settings_.get() == nullptr)
  {
    settings_ = Settings::create();
  }
}

// -----------------------------------------------------------------------------

auto Base::operator=(const Base & src) -> Base &
{
  if(settings_.get() != src.settings_.get())
  {
    auto lk = begin_unique(); // write lock

    settings_ = src.settings_;
  }

  return *this;
}

// -----------------------------------------------------------------------------

Base::~Base()
{
}

// -----------------------------------------------------------------------------

auto Base::get_ptr() const -> const pSettings &
{
  return settings_;
}

// -----------------------------------------------------------------------------

auto Base::server_addr() const -> Addr
{
  auto lk = begin_shared(); // read lock

  return Addr{settings_->local_base_};
}

// -----------------------------------------------------------------------------

auto Base::begin_shared() const -> std::shared_lock<std::shared_mutex>
{
  return std::shared_lock{mutex_};
}

// -----------------------------------------------------------------------------

auto Base::begin_unique() const -> std::unique_lock<std::shared_mutex>
{
  return std::unique_lock{mutex_};
}

// -----------------------------------------------------------------------------

void Base::log(LogLvl lvl, std::string_view msg) const
{
  auto lk = begin_shared(); // read lock

  if((int)lvl <= settings_->use_syslog)
  {
    syslog((int)lvl,
           "[%d] %s %s",
           (int)syscall(SYS_gettid),
           to_string(lvl).data(),
           msg.data());
  }

  if(settings_->log_) settings_->log_(lvl, msg);
}

// -----------------------------------------------------------------------------

void Base::set_logger(fLogMsg new_logger)
{
  auto lk = begin_unique(); // write lock

  settings_->log_ = new_logger;
}

// -----------------------------------------------------------------------------

auto Base::get_root_dir() const -> std::string
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

auto Base::get_lib_dir() const -> std::string
{
  auto lk = begin_shared(); // read lock

  bool fin_slash =
      settings_->lib_dir.size() &&
      (*--settings_->lib_dir.end() == '/');

  return settings_->lib_dir + (fin_slash ? "" : "/");
}

// -----------------------------------------------------------------------------

auto Base::get_lib_name_fb() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->lib_name};
}

// -----------------------------------------------------------------------------

auto Base::get_retransmit_count() const -> uint16_t
{
  auto lk = begin_shared(); // read lock

  return settings_->retransmit_count_;
}

// -----------------------------------------------------------------------------

auto Base::get_connection() const
  -> std::tuple<std::string,
                std::string,
                std::string,
                std::string,
                uint16_t>
{
  auto lk = begin_shared(); // read lock

  return {std::string{settings_->db},
          std::string{settings_->user},
          std::string{settings_->pass},
          std::string{settings_->role},
          settings_->dialect};
}

// -----------------------------------------------------------------------------

auto Base::get_local_base_str() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return settings_->local_base_.str();
}
// -----------------------------------------------------------------------------

bool Base::get_is_daemon() const
{
  auto lk = begin_shared(); // read lock

  return settings_->is_daemon;
}

// -----------------------------------------------------------------------------

auto Base::get_serach_dir() const -> VecStr
{

  auto lk = begin_shared(); // read lock

  VecStr ret;
  for(const auto & one_dir: settings_->backup_dirs)
  {
    ret.emplace_back(one_dir);
  }

  return ret;
}

// -----------------------------------------------------------------------------

bool Base::load_options(int argc, char* argv[])
{
  auto lk = begin_unique(); // write lock

  return settings_->load_options(argc, argv);
}

// -----------------------------------------------------------------------------

void Base::out_help(std::ostream & stream, std::string_view app) const
{
  auto lk = begin_shared(); // read lock

  settings_->out_help(stream, app);
}

// -----------------------------------------------------------------------------

void Base::out_id(std::ostream & stream) const
{
  auto lk = begin_shared(); // read lock

  settings_->out_id(stream);
}

// -----------------------------------------------------------------------------

int Base::get_file_chmod() const
{
  auto lk = begin_shared(); // read lock

  return settings_->file_chmod;
}

auto Base::get_file_chown_user() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->file_chown_user};
}

auto Base::get_file_chown_grp() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->file_chown_grp};
}


} // namespace tftp
