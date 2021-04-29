/**
 * \file tftpBase.h
 * \brief Base TFTP server class module
 *
 *  Base class for TFTP server
 *
 *  License GPL-3.0
 *
 *  \date   05-apr-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.1
 */

#include <fstream>
#include <regex>
#include <unistd.h>
#include <sys/syscall.h>
#include <arpa/inet.h>
#include <getopt.h>
#include <syslog.h>

#include "tftpBase.h"



namespace tftp
{

// -----------------------------------------------------------------------------

Base::Base():
    settings_{Settings::create()}
{
  // Get system library path from current maps
  std::ifstream maps;
  maps.open("/proc/self/maps", std::ios_base::in);
  for (Buf arr(4096, 0); maps.getline(arr.data(), arr.size(), '\n'); )
  {
    std::string line{arr.data()};
    std::regex  re("(/.*/)(libc-)");
    std::smatch sm;

    if(std::regex_search(line, sm, re))
    {
      settings_->lib_dir.assign("/usr").append(sm[1].str());  // hack
      break;
    }
  }

  if(!settings_->lib_dir.size()) // last chance try (life hack)
  {
    settings_->lib_dir.assign("/usr/lib/x86_64-linux-gnu");
  }

  maps.close();
}

// -----------------------------------------------------------------------------

Base::Base(const Base & src):
    settings_{src.settings_}
{
}

// -----------------------------------------------------------------------------

Base::Base(Base && src):
    settings_{std::move(src.settings_)}
{
}

// -----------------------------------------------------------------------------

Base & Base::operator=(Base && src)
{
  settings_ = std::move(src.settings_);

  return *this;
}

// -----------------------------------------------------------------------------

Base::~Base()
{
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
/*
void Base::set_syslog_level(const int lvl)
{
  auto lk = begin_unique(); // write lock

  settings_->use_syslog = lvl;
}
// -----------------------------------------------------------------------------

void Base::set_root_dir(std::string_view root_dir)
{
  auto lk = begin_unique(); // write lock

  settings_->root_dir.assign(root_dir);
}
*/
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
/*
void Base::set_lib_dir(std::string_view dir)
{
  auto lk = begin_unique(); // write lock

  settings_->lib_dir.assign(dir);
}
*/
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
/*
void Base::set_lib_name_fb(std::string_view fb_name)
{
  auto lk = begin_unique(); // write lock

  settings_->lib_name.assign(fb_name);
}
*/
// -----------------------------------------------------------------------------

auto Base::get_lib_name_fb() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->lib_name};
}

// -----------------------------------------------------------------------------
/*
void Base::set_connection_db  (std::string_view val)
{ auto lk = begin_unique(); settings_->db  .assign(val); }

void Base::set_connection_user(std::string_view val)
{ auto lk = begin_unique(); settings_->user.assign(val); }

void Base::set_connection_pass(std::string_view val)
{ auto lk = begin_unique(); settings_->pass.assign(val); }

void Base::set_connection_role(std::string_view val)
{ auto lk = begin_unique(); settings_->role.assign(val); }

void Base::set_connection_dialect(uint16_t      val)
{ auto lk = begin_unique(); settings_->dialect = val; }

// -----------------------------------------------------------------------------

void Base::set_connection(
    std::string_view db,
    std::string_view user,
    std::string_view pass,
    std::string_view role,
    uint16_t         dialect)
{
  set_connection_db  (db);
  set_connection_user(user);
  set_connection_pass(pass);
  set_connection_role(role);
  set_connection_dialect(dialect);
}

// -----------------------------------------------------------------------------

void Base::set_retransmit_count(const uint16_t & val)
{
  settings_->retransmit_count_ = val;
}
*/

auto Base::get_retransmit_count() const -> const uint16_t &
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

auto Base::local_base() -> Addr &
{
  return settings_->local_base_;
}

auto Base::local_base() const -> const Addr &
{
  return settings_->local_base_;
}

// -----------------------------------------------------------------------------

auto Base::get_local_base_str() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return settings_->local_base_.str();
}
// -----------------------------------------------------------------------------
/*
void Base::set_is_daemon(bool value)
{
  auto lk = begin_unique(); // write lock

  settings_->is_daemon = value;
}
*/
// -----------------------------------------------------------------------------

bool Base::get_is_daemon() const
{
  auto lk = begin_shared(); // read lock

  return settings_->is_daemon;
}

// -----------------------------------------------------------------------------
/*
void Base::set_search_dir_append(std::string_view new_dir)
{
  auto lk = begin_unique(); // write lock

  bool need_append=true;
  for(auto & item: settings_->backup_dirs)
  {
    if(item == new_dir) { need_append=false; break; }
  }

  if(need_append) settings_->backup_dirs.emplace_back(new_dir);
}
*/
// -----------------------------------------------------------------------------

auto Base::get_serach_dir() const -> std::vector<std::string>
{

  auto lk = begin_shared(); // read lock

  std::vector<std::string> ret;
  for(const auto & one_dir: settings_->backup_dirs)
  {
    ret.emplace_back(one_dir);
  }

  return ret;
}

// -----------------------------------------------------------------------------
/*
void Base::set_local_base(std::string_view addr)
{
  auto lk = begin_unique(); // write lock

  settings_->local_base_.clear();

  auto [set_addr,set_port] = settings_->local_base_.set_string(addr);

  if(!set_addr) settings_->local_base_.set_family(AF_INET);
  if(!set_port) settings_->local_base_.set_port(constants::default_tftp_port);
}
*/
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

} // namespace tftp
