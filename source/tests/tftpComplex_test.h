/*
 * tftpComplex_test.h
 *
 *  Created on: 30 сент. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TESTS_TFTPCOMPLEX_TEST_H_
#define SOURCE_TESTS_TFTPCOMPLEX_TEST_H_

namespace ComplexTest
{

//------------------------------------------------------------------------------

using LogVector = std::array<size_t, 7U>; //unit_tests::FakeLog<7>;

using ClientCase = std::tuple<tftp::VecStr, LogVector, size_t>;

using ClientCases = std::vector<ClientCase>;

//------------------------------------------------------------------------------

using Logger = unit_tests::FakeLog<7>;

struct ClientData
{
  Logger               log{false};
  tftp::pClientSession p_ses{nullptr};
  std::thread          thr{};
};

using ClientsData = std::vector<ClientData>;

//------------------------------------------------------------------------------

class TestServer
{
protected:
  bool valid_;
  uint16_t port_;

  Logger logger_;
  tftp::pSrv srv_;

  std::string      main_filename_;
  filesystem::path main_file_;

  std::string listening_;

  std::thread thr_;

public:

  TestServer():
      valid_{false},
      port_{unit_tests::gen_test_port()},
      logger_{},
      srv_{nullptr},
      main_filename_{unit_tests::gen_file_name(0U)},
      main_file_{},
      listening_{"0.0.0.0"},
      thr_{}
  {
    // check testing server port valid
    if(!(valid_ = (port_ >= unit_tests::port_min) && (port_ <= unit_tests::port_max))) return;
    listening_.append(":").append(std::to_string(port_));

    // local dir prepare
    if(!(valid_ = unit_tests::check_local_directory())) return;

    // get source filename
    filesystem::path src_file{filesystem::current_path()};
//    src_file /= "bin";
//    src_file /=  tftp::constants::app_srv_name;
    src_file /= "Makefile";
    if(!(valid_ = filesystem::exists(src_file))) return;

    // destination main test filename
    main_file_ = unit_tests::local_dir;
    main_file_ /= main_filename_;

    // Copy main test file to testing dir
    if(!(valid_ = filesystem::copy_file(src_file, main_file_))) return;
    if(!(valid_ = filesystem::exists(main_file_))) return;

    // server logger helper
    auto cb_syslog_srv = std::bind(
        & unit_tests::FakeLog<7>::syslog,
        & logger_,
        std::placeholders::_1,
        std::placeholders::_2);

    // server arguments
    const char * srv_args[]=
    {
      "./server-fw",
      "--root-dir", unit_tests::local_dir.c_str(),
      "--daemon",
      "--syslog", "7",
      "--file-chuser", "usr",
      "--file-chgrp", "grp",
      "--file-chmod", "0666",
      listening_.c_str()
    };

    // server arg parser construct
    tftp::ArgParser ap_srv{tftp::constants::srv_arg_settings};

    // parse arguments
    ap_srv.run(
        cb_syslog_srv,
        sizeof(srv_args)/sizeof(srv_args[0]),
        const_cast<char **>(srv_args));
    if(!(valid_ = logger_.chk_clear({0,0,0,0,0,0,9}))) return;

    // server settings
    tftp::SrvSettings srv_st;
    if(!(valid_ = (srv_st.load_options(cb_syslog_srv, ap_srv) == tftp::TripleResult::ok))) return;
    if(!(valid_ = logger_.chk_clear({0,0,0,0,0,0,2}))) return;

    // server instance
    srv_ = std::move(tftp::Srv::create(cb_syslog_srv, srv_st));
    if(!(valid_ = srv_.get())) return;

    // server init
    if(!(valid_ = srv_->init(listening_))) return;
    if(!(valid_ = logger_.chk_clear({0,0,0,0,0,3,0}))) return;

    // run thread
    thr_ = std::thread{& tftp::Srv::main_loop, srv_.get()};

  };

  ~TestServer()
  {
    if(srv_.get())
    {
      if(!srv_->is_stopped())
      {
        srv_->stop();
        usleep(1000000);
      }

      if(thr_.joinable()) thr_.join();
    }

    //unit_tests::files_delete();
  };

  bool valid() const { return valid_; };

  auto srv() -> tftp::pSrv & { return srv_; };

  auto main_filename() const -> const std::string & { return main_filename_; };

  auto listening() const -> const std::string & { return listening_; };

  auto main_file() const -> const filesystem::path & { return main_file_; };

  auto logger() const -> const Logger & { return logger_; };

  auto root_dir() const -> const filesystem::path & { return unit_tests::local_dir; };


};





//------------------------------------------------------------------------------

} // namespace ComplexTest

#endif /* SOURCE_TESTS_TFTPCOMPLEX_TEST_H_ */
