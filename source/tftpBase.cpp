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
  local_base_as_inet().sin_family = AF_INET;
  local_base_as_inet().sin_port   = htobe16(default_tftp_port);

  settings_->lib_name.assign(default_fb_lib_name);
  settings_->dialect = default_fb_dialect;
  settings_->use_syslog = default_tftp_syslog_lvl;

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

Base::Base(Base & src):
    settings_{src.settings_}
{
}

// -----------------------------------------------------------------------------

Base::Base(Base && src):
    settings_{std::move(src.settings_)}
{
}

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

void Base::set_syslog_level(const int lvl)
{
  auto lk = begin_unique(); // write lock

  settings_->use_syslog = lvl;
}

auto Base::get_syslog_level() const -> int
{
  auto lk = begin_shared(); // read lock

  return settings_->use_syslog;
}

// -----------------------------------------------------------------------------

void Base::set_root_dir(std::string_view root_dir)
{
  auto lk = begin_unique(); // write lock

  settings_->root_dir.assign(root_dir);
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

void Base::set_lib_dir(std::string_view dir)
{
  auto lk = begin_unique(); // write lock

  settings_->lib_dir.assign(dir);
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

void Base::set_lib_name_fb(std::string_view fb_name)
{
  auto lk = begin_unique(); // write lock

  settings_->lib_name.assign(fb_name);
}

// -----------------------------------------------------------------------------

auto Base::get_lib_name_fb() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return std::string{settings_->lib_name};
}

// -----------------------------------------------------------------------------

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
auto Base::local_base_as_inet() -> struct sockaddr_in &
{
  if(settings_->local_base_.empty())
  {
    settings_->local_base_.assign(sizeof(struct sockaddr_in), 0);
  }
  else
  {
    if(settings_->local_base_.size() < sizeof(struct sockaddr_in))
      ERROR_THROW_RUNTIME(
          "Small size settings_->local_base_ ("+
          std::to_string(settings_->local_base_.size())+
          " bytes less struct sockaddr_in)");
  }

  return *((struct sockaddr_in *) settings_->local_base_.data());
}

// -----------------------------------------------------------------------------

auto Base::local_base_as_inet6() -> struct sockaddr_in6 &
{
  if(settings_->local_base_.empty())
  {
    settings_->local_base_.assign(sizeof(struct sockaddr_in6), 0);
  }
  else
  {
    if(settings_->local_base_.size() < sizeof(struct sockaddr_in6))
      ERROR_THROW_RUNTIME(
          "Small size settings_->local_base_ ("+
          std::to_string(settings_->local_base_.size())+
          " bytes less struct sockaddr_in6)");
  }

  return *((struct sockaddr_in6 *) settings_->local_base_.data());
}

// -----------------------------------------------------------------------------

void Base::set_local_base_inet(struct in_addr * addr, uint16_t port)
{
  auto lk = begin_unique(); // write lock

  settings_->local_base_.clear();

  local_base_as_inet().sin_family = AF_INET;
  local_base_as_inet().sin_addr   = * addr;
  local_base_as_inet().sin_port   = htobe16(port);
}

// -----------------------------------------------------------------------------

void Base::set_local_base_inet6(struct in6_addr * addr, uint16_t port)
{
  auto lk = begin_unique(); // write lock

  settings_->local_base_.clear();

  local_base_as_inet6().sin6_family = AF_INET6;
  local_base_as_inet6().sin6_addr   = * addr;
  local_base_as_inet6().sin6_port  = htobe16(port);
}

// -----------------------------------------------------------------------------

auto Base::get_local_base_str() const -> std::string
{
  auto lk = begin_shared(); // read lock

  return sockaddr_to_str(
      settings_->local_base_.cbegin(),
      settings_->local_base_.cend());
}

// -----------------------------------------------------------------------------

void Base::set_is_daemon(bool value)
{
  auto lk = begin_unique(); // write lock

  settings_->is_daemon = value;
}

// -----------------------------------------------------------------------------

bool Base::get_is_daemon() const
{
  auto lk = begin_shared(); // read lock

  return settings_->is_daemon;
}

// -----------------------------------------------------------------------------

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

void Base::set_local_base(std::string_view addr)
{
  auto lk = begin_unique(); // write lock

  settings_->local_base_.clear();

  std::string tmp{addr.begin(), addr.end()};
  std::regex  re4("^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})$|(^(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}):(\\d{0,5}))$|^:(\\d{1,5})$");
  std::regex  re6("^\\[?([0-9a-fA-F\\:]*)[\\]]?:(\\d{1,5})$|^\\[?([0-9a-fA-F\\:]*)[\\]]?$");
  std::smatch sm4, sm6;

  bool is_ipv4 = (std::regex_search(tmp, sm4, re4) && (sm4.size() == 6));
  bool is_ipv6 = std::regex_search(tmp, sm6, re6);

  auto str_to_port_be=[](const std::string & src) -> uint16_t
      {
        in_port_t port=0;
        try { port = (std::stoul(src) & 0x0000FFFFU); } catch (...) {};
        if(!port) port = default_tftp_port;
        return htobe16(port);
      };

  if(is_ipv4)
  {
    // family
    local_base_as_inet().sin_family = AF_INET;

    // port
    local_base_as_inet().sin_port = htobe16(default_tftp_port);
    if(std::string port_s{sm4[5].str()}; port_s.size())
    {
      local_base_as_inet().sin_port = str_to_port_be(port_s);
    }
    else
    if(std::string port_s{sm4[4].str()}; port_s.size())
    {
      local_base_as_inet().sin_port = str_to_port_be(port_s);
    }

    // addr
    if(std::string addr_s{sm4[3].str()}; addr_s.size())
    {
      inet_pton(AF_INET, addr_s.c_str(), & local_base_as_inet().sin_addr);
    }
    else
    if(std::string addr_s{sm4[1].str()}; addr_s.size())
    {
      inet_pton(AF_INET, addr_s.c_str(), & local_base_as_inet().sin_addr);
    }
  }
  else
  if(is_ipv6)
  {
    // family
    local_base_as_inet6().sin6_family = AF_INET6;

    // port
    local_base_as_inet6().sin6_port = htobe16(default_tftp_port);
    if(std::string port_s{sm6[2].str()}; port_s.size())
    {
      local_base_as_inet6().sin6_port = str_to_port_be(port_s);
    }

    // addr
    if(std::string addr_s{sm6[3].str()}; addr_s.size())
    {
      inet_pton(AF_INET6, addr_s.c_str(), & local_base_as_inet6().sin6_addr);
    }
    else
    if(std::string addr_s{sm6[1].str()}; addr_s.size())
    {
      inet_pton(AF_INET6, addr_s.c_str(), & local_base_as_inet6().sin6_addr);
    }
  }
  else
  {

  }
}

// -----------------------------------------------------------------------------

bool Base::load_options(int argc, char* argv[])
{
  bool ret = true;

  static const char *optString = "l:L:s:S:h?";

  static const struct option longOpts[] =
  {
      { "listen",     required_argument, NULL, 'l' }, // 0
      { "ip",         required_argument, NULL, 'l' }, // 1
      { "syslog",     required_argument, NULL, 's' }, // 2
      { "help",             no_argument, NULL, 'h' }, // 3
      { "lib-dir",    required_argument, NULL,  0  }, // 4
      { "lib-name",   required_argument, NULL,  0  }, // 5
      { "root-dir",   required_argument, NULL,  0  }, // 6
      { "search",     required_argument, NULL,  0  }, // 7
      { "fb-db",      required_argument, NULL,  0  }, // 8
      { "fb-user",    required_argument, NULL,  0  }, // 9
      { "fb-pass",    required_argument, NULL,  0  }, // 10
      { "fb-role",    required_argument, NULL,  0  }, // 11
      { "fb-dialect", required_argument, NULL,  0  }, // 12
      { "daemon",           no_argument, NULL,  0  }, // 13
      { NULL,               no_argument, NULL,  0  }  // always last
  };

  set_search_dir();

  optind=1;
  while(argc > 1)
  {
    int longIndex;
    int opt = getopt_long(argc, argv, optString, longOpts, & longIndex);
    if(opt == -1) break; // end parsing
    switch(opt)
    {
    case 'l':
      if(optarg) set_local_base(optarg);
      break;
    case 's':
    case 'S':
      {
        if(optarg)
        {
          try
          {
            int lvl = LOG_PRI(std::stoi(optarg));
            set_syslog_level(lvl);
          } catch (...) { };
        }
      }
      break;
    case 'h':
    case 'H':
    case '?':
      ret = false; // Help message cout
      break;
    case 0:

      switch(longIndex)
      {
      case 4: // --lib-dir
        if(optarg) set_lib_dir(optarg);
        break;
      case 5: // --lib-name
        if(optarg) set_lib_name_fb(optarg);
        break;
      case 6: // --root-dir
        if(optarg) set_root_dir(optarg);
        break;
      case 7: // --search
        if(optarg) set_search_dir_append(optarg);
        break;
      case 8: // --fb-db
        if(optarg) set_connection_db(optarg);
        break;
      case 9: // --fb-user
        if(optarg) set_connection_user(optarg);
        break;
      case 10: // --fb-pass
        if(optarg) set_connection_pass(optarg);
        break;
      case 11: // --fb-role
        if(optarg) set_connection_role(optarg);
        break;
      case 12: // --fb-dialect
        if(optarg)
        {
          try
          {
            uint16_t dial = (std::stoul(optarg) & 0xFFFFU);
            set_connection_dialect(dial);
          } catch (...) { };
        }
        break;
      case 13: // --daemon
        set_is_daemon(true);
        break;
      } // case
      break;
    } // case
  }

  return ret;
}

// -----------------------------------------------------------------------------

void Base::out_help(std::ostream & stream, std::string_view app) const
{
  out_id(stream);

  stream << "Some features:" << std::endl
  << "  - Recursive search requested files by md5 sum in search directory" << std::endl
  << "  - Use Firebird SQL server as file storage (optional requirement)" << std::endl
  << "Usage: " << app << " [<option1> [<option1 argument>]] [<option2> [<option2 argument>]] ... " << std::endl
  << "Possible options:" << std::endl
  << "  {-h|-H|-?|--help} Show help message" << std::endl
  << "  {-l|-L|--ip|--listen} {<IPv4>|[<IPv6>]}[:port] Listening address and port" << std::endl
  << "    (default 0.0.0.0:" << default_tftp_port << ")" << std::endl
  << "    Sample IPv4: 192.168.0.1:69" << std::endl
  << "    Sample IPv6: [::1]:69" << std::endl
  << "  {-s|-S|--syslog} <0...7> SYSLOG level flooding (default " << default_tftp_syslog_lvl << ")" << std::endl
  << "  --lib-dir <directory> System library directory" << std::endl
  << "  --lib-name <name> Firebird library filename (default " << default_fb_lib_name << ")" << std::endl
  << "  --root-dir <directory> Server root directory" << std::endl
  << "  --search <directory> Directory for recursive search by md5 sum (may be much)" << std::endl
  << "  --fb-db <database> Firebird access database name" << std::endl
  << "  --fb-user <username> Firebird access user name" << std::endl
  << "  --fb-pass <password> Firebird access password" << std::endl
  << "  --fb-role <role> Firebird access role" << std::endl
  << "  --fb-dialect <1...3> Firebird server dialect (default " << default_fb_dialect << ")" << std::endl
  << "  --daemon Run as daemon" << std::endl;
}

// -----------------------------------------------------------------------------

void Base::out_id(std::ostream & stream) const
{
  stream << "Simple tftp firmware server 'server_fw' licensed GPL-3.0" << std::endl
  << "(c) 2019-2021 Vitaliy.V.Shirinkin, e-mail: vitaliy.shirinkin@gmail.com" << std::endl;
}

// -----------------------------------------------------------------------------

} // namespace tftp
