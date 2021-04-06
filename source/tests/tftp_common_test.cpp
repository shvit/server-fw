//#include <type_traits>
#include <utility>
//#include <vector>
#include <iostream>

#include "../tftp_common.h"
#include "../tftpBase.h"
#include "test.h"     

UNIT_TEST_SUITE_BEGIN(tftp_common)


class tst_Base: public tftp::Base
{
public:
  tftp::pSettings get_settings() { return settings_; };

  decltype(auto) tst_local_base_as_inet () { return local_base_as_inet (); };
  decltype(auto) tst_local_base_as_inet6() { return local_base_as_inet6(); };

  template<typename T, typename ... Ts>
  decltype(auto) get_buf_item_raw(Ts && ... args) { return Base::get_buf_item_raw<T>(std::forward<Ts>(args) ...); }

  template<typename T, typename ... Ts>
  decltype(auto) get_buf_item_ntoh(Ts && ... args) { return Base::get_buf_item_ntoh<T>(std::forward<Ts>(args) ...); }

  template<typename ... Ts>
  decltype(auto) set_buf_item_raw(Ts && ... args) { return Base::set_buf_item_raw(std::forward<Ts>(args) ...); }

  template<typename ... Ts>
  decltype(auto) set_buf_item_hton(Ts && ... args) { return Base::set_buf_item_hton(std::forward<Ts>(args) ...); }

  template<typename ... Ts>
  decltype(auto) set_buf_cont_str(Ts && ... args) { return Base::set_buf_cont_str(std::forward<Ts>(args) ...); }

  //template<typename T>
  //auto get_buf_item_raw(tftp::Buf & buf, const tftp::buffer_size_t offset) const
    //-> std::enable_if_t<std::is_integral_v<T>, T &> { return base::get_buf_item_raw<T>(buf, offset); };

  //template<typename T>
  //auto get_buf_item_ntoh(tftp::Buf & buf, const tftp::buffer_size_t offset) const
    //-> std::enable_if_t<std::is_integral_v<T>, T> { return base::get_buf_item_ntoh<T>(buf, offset); };

  //template<typename T>
  //auto set_buf_item_raw(tftp::Buf & buf, const tftp::buffer_size_t offset, const T value)
    //-> std::enable_if_t<std::is_integral_v<T>, void> { base::set_buf_item_raw<T>(buf, offset, value); };

  //template<typename T>
  //auto set_buf_item_hton(tftp::Buf & buf, const tftp::buffer_size_t offset, const T value)
    //-> std::enable_if_t<std::is_integral_v<T>, void> { base::set_buf_item_hton<T>(buf, offset, value); };



};

//---------------------------------------------------------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(buffer_operations, "Class 'tftp::Base' - buffer operations")
  tst_Base b;

  tftp::Buf a1{0x00U, 0x01U, 0x02U, 0x03U, 0x04U, 0x05U, 0x06U, 0x07U,
                    0x08U, 0x09U, 0x0aU, 0x0bU, 0x0cU, 0x0dU, 0x0eU, 0x0fU};

  // 2 bytes
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint16_t>(a1, 0) == 0x0100U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint16_t>(a1, 0) == 0x0001U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint16_t>(a1, 1) == 0x0201U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint16_t>(a1, 1) == 0x0102U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint16_t>(a1, 2) == 0x0302U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint16_t>(a1, 2) == 0x0203U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint16_t>(a1, 3) == 0x0403U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint16_t>(a1, 3) == 0x0304U);
  b.get_buf_item_raw<uint16_t>(a1, 0) = 0x1122U;
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[0])==0x22U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[1])==0x11U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[2])==0x02U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[3])==0x03U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint64_t>(a1, 0) == 0x2211020304050607U);
  TEST_CHECK_TRUE(b.set_buf_item_raw(a1, 1, (uint16_t) 0x3344U) == 2U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[0])==0x22U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[1])==0x44U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[2])==0x33U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[3])==0x03U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint64_t>(a1, 0) == 0x2244330304050607U);
  TEST_CHECK_TRUE(b.set_buf_item_hton(a1, 0, (uint16_t) 0xf1f2U) == 2U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[0])==0xf1U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[1])==0xf2U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[2])==0x33U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[3])==0x03U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint64_t>(a1, 0) == 0xf1f2330304050607U);
  TEST_CHECK_TRUE(b.set_buf_item_hton(a1, 1, (uint16_t) 0xf3f4U) == 2U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[0])==0xf1U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[1])==0xf3U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[2])==0xf4U);
  TEST_CHECK_TRUE(static_cast<uint8_t>(a1[3])==0x03U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint64_t>(a1, 0) == 0xf1f3f40304050607U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint64_t>(a1, 0) == 0x0706050403f4f3f1U);
  TEST_CHECK_TRUE(b.set_buf_item_hton(a1, 8, (uint64_t) 0x8877665544332211U) == 8U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint64_t>(a1, 0) == 0xf1f3f40304050607U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint64_t>(a1, 0) == 0x0706050403f4f3f1U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint64_t>(a1, 8) == 0x8877665544332211U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint64_t>(a1, 8) == 0x1122334455667788U);

  // write string
  std::string s{"1234567890abcdef"};
  TEST_CHECK_TRUE(b.set_buf_cont_str(a1, 0, s, false) == 16U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint64_t>(a1, 0) == 0x3837363534333231U);
  TEST_CHECK_TRUE(b.get_buf_item_raw <uint64_t>(a1, 8) == 0x6665646362613039U);
  CHK_EXCEPTION__INV_ARG(b.set_buf_cont_str(a1, 0, s, true));

  TEST_CHECK_TRUE(b.set_buf_cont_str(a1, 0, std::string{"qrs"}, true) == 4U);
  TEST_CHECK_TRUE(b.set_buf_cont_str(a1, 4, std::string{"tuv"}, true) == 4U);
  TEST_CHECK_TRUE(b.set_buf_cont_str(a1, 0, std::string{""}, true) == 0U);
  TEST_CHECK_TRUE(b.set_buf_cont_str(a1, 4, std::string{""}, true) == 0U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint32_t>(a1, 0) == 0x71727300U);
  TEST_CHECK_TRUE(b.get_buf_item_ntoh<uint32_t>(a1, 4) == 0x74757600U);
//
UNIT_TEST_CASE_END

//---------------------------------------------------------------------------------------------------------------------------------

UNIT_TEST_CASE_BEGIN(base_load_options, "Class 'tftp::Base' - options")

  // 1
  START_ITER("default options");
  {
    tst_Base b;
    // is daemon
    TEST_CHECK_FALSE(b.get_settings()->is_daemon);
    TEST_CHECK_FALSE(b.get_is_daemon());
    // syslog level
    TEST_CHECK_TRUE(b.get_settings()->use_syslog == 6);
    TEST_CHECK_TRUE(b.get_syslog_level() == 6);
    // addr
    TEST_CHECK_TRUE(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_family == 2);
    TEST_CHECK_TRUE(be16toh(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_port) == 69);
    TEST_CHECK_TRUE(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_addr.s_addr == 0);
    TEST_CHECK_TRUE(b.tst_local_base_as_inet().sin_family == 2);
    TEST_CHECK_TRUE(be16toh(b.tst_local_base_as_inet().sin_port) == 69);
    TEST_CHECK_TRUE(b.tst_local_base_as_inet().sin_addr.s_addr == 0);
    TEST_CHECK_TRUE(b.get_local_base_str() == "0.0.0.0:69");
    TEST_CHECK_TRUE(& ((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_addr.s_addr == & b.tst_local_base_as_inet().sin_addr.s_addr);

    // sys lib dir
    TEST_CHECK_TRUE(b.get_settings()->lib_dir == "/usr/lib/x86_64-linux-gnu/");
    TEST_CHECK_TRUE(b.get_lib_dir() == "/usr/lib/x86_64-linux-gnu/");
    TEST_CHECK_TRUE(b.get_lib_dir().data() != b.get_settings()->lib_dir.data());
    // fb lib name
    TEST_CHECK_TRUE(b.get_settings()->lib_name == "libfbclient.so");
    TEST_CHECK_TRUE(b.get_lib_name_fb() == "libfbclient.so");
    TEST_CHECK_TRUE(b.get_lib_name_fb().data() != b.get_settings()->lib_name.data());
    // root dir
    TEST_CHECK_TRUE(b.get_settings()->root_dir == "");
    TEST_CHECK_TRUE(b.get_root_dir() == "");
    TEST_CHECK_TRUE(b.get_root_dir().data() != b.get_settings()->root_dir.data());
    // search dirs
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs.size() == 0);
    // fb account
    TEST_CHECK_TRUE(b.get_settings()->db == "");
    TEST_CHECK_TRUE(b.get_settings()->user == "");
    TEST_CHECK_TRUE(b.get_settings()->pass == "");
    TEST_CHECK_TRUE(b.get_settings()->role == "");
    TEST_CHECK_TRUE(b.get_settings()->dialect == 3);
    auto [db1,u1,p1,r1,d1] = b.get_connection();
    TEST_CHECK_TRUE(db1 == "");
    TEST_CHECK_TRUE(u1 == "");
    TEST_CHECK_TRUE(p1 == "");
    TEST_CHECK_TRUE(r1 == "");
    TEST_CHECK_TRUE(d1 == 3);
    TEST_CHECK_TRUE(db1.data() != b.get_settings()->db.data());
    TEST_CHECK_TRUE(u1.data()  != b.get_settings()->user.data());
    TEST_CHECK_TRUE(p1.data()  != b.get_settings()->pass.data());
    TEST_CHECK_TRUE(r1.data()  != b.get_settings()->role.data());
  }



  // 2
  START_ITER("load options");
  {
    const char * tst_args[]=
      {
          "./server_fw", "--daemon", "--syslog", "7",
          "--ip", "1.1.1.1:7777",
          "--root-dir", "/mnt/tftp", "--search", "/mnt/tst1", "--search", "/mnt/tst2", "--search", "/mnt/tst3",
          "--fb-db", "tester.fdb", "--fb-user", "SYSDBA", "--fb-pass", "masterkey", "--fb-role", "none", "--fb-dialect", "3",
          "--lib-dir", "/tmp/libs", "--lib-name", "fbclient"
      };

    tst_Base b;
    b.load_options(sizeof(tst_args)/sizeof(tst_args[0]), const_cast<char **>(tst_args));
    // is daemon
    TEST_CHECK_TRUE(b.get_settings()->is_daemon);
    TEST_CHECK_TRUE(b.get_is_daemon());
    // syslog level
    TEST_CHECK_TRUE(b.get_settings()->use_syslog == 7);
    TEST_CHECK_TRUE(b.get_syslog_level() == 7);
    // addr
    TEST_CHECK_TRUE(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_family == 2);
    TEST_CHECK_TRUE(be16toh(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_port) == 7777);
    TEST_CHECK_TRUE(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_addr.s_addr == 16843009);
    TEST_CHECK_TRUE(b.tst_local_base_as_inet().sin_family == 2);
    TEST_CHECK_TRUE(be16toh(b.tst_local_base_as_inet().sin_port) == 7777);
    TEST_CHECK_TRUE(b.tst_local_base_as_inet().sin_addr.s_addr == 16843009);
    TEST_CHECK_TRUE(b.get_local_base_str() == "1.1.1.1:7777");
    TEST_CHECK_TRUE(& ((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_addr.s_addr == & b.tst_local_base_as_inet().sin_addr.s_addr);
    // sys lib dir
    TEST_CHECK_TRUE(b.get_settings()->lib_dir == "/tmp/libs");
    TEST_CHECK_TRUE(b.get_lib_dir() == "/tmp/libs/");
    TEST_CHECK_TRUE(b.get_lib_dir().data() != b.get_settings()->lib_dir.data());
    // fb lib name
    TEST_CHECK_TRUE(b.get_settings()->lib_name == "fbclient");
    TEST_CHECK_TRUE(b.get_lib_name_fb() == "fbclient");
    TEST_CHECK_TRUE(b.get_lib_name_fb().data() != b.get_settings()->lib_name.data());
    // root dir
    TEST_CHECK_TRUE(b.get_settings()->root_dir == "/mnt/tftp");
    TEST_CHECK_TRUE(b.get_root_dir() == "/mnt/tftp/");
    TEST_CHECK_TRUE(b.get_root_dir().data() != b.get_settings()->root_dir.data());
    // search dirs
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs.size() == 3);
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs[0] == "/mnt/tst1");
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs[1] == "/mnt/tst2");
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs[2] == "/mnt/tst3");
    {
      auto lst = b.get_serach_dir();
      for(size_t iter=0; iter < lst.size(); ++iter)
      {
        TEST_CHECK_TRUE(lst[iter] == std::string{"/mnt/tst"}.append(std::to_string(iter+1)));
      }
    }
    // fb account
    TEST_CHECK_TRUE(b.get_settings()->db == "tester.fdb");
    TEST_CHECK_TRUE(b.get_settings()->user == "SYSDBA");
    TEST_CHECK_TRUE(b.get_settings()->pass == "masterkey");
    TEST_CHECK_TRUE(b.get_settings()->role == "none");
    TEST_CHECK_TRUE(b.get_settings()->dialect == 3);
    {
      auto [db1,u1,p1,r1,d1] = b.get_connection();
      TEST_CHECK_TRUE(db1 == "tester.fdb");
      TEST_CHECK_TRUE(u1 == "SYSDBA");
      TEST_CHECK_TRUE(p1 == "masterkey");
      TEST_CHECK_TRUE(r1 == "none");
      TEST_CHECK_TRUE(d1 == 3);
      TEST_CHECK_TRUE(db1.data() != b.get_settings()->db.data());
      TEST_CHECK_TRUE(u1.data()  != b.get_settings()->user.data());
      TEST_CHECK_TRUE(p1.data()  != b.get_settings()->pass.data());
      TEST_CHECK_TRUE(r1.data()  != b.get_settings()->role.data());
    }

    // 3
    START_ITER("set options");
    b.set_syslog_level(5);
    b.set_is_daemon(false);
    b.set_root_dir("/tmp/qwerty");
    b.set_search_dir("/var/ququ/dir1", "/var/ququ/dir2");
    b.set_lib_dir("/bubin/libs");
    b.set_lib_name_fb("ib_client");
    b.set_connection("qq.gdb", "admin", "passss","no_role", 2);
    struct in_addr a{0x02020202U};
    b.set_local_base_inet(& a, 8888);
    // is daemon
    TEST_CHECK_FALSE(b.get_settings()->is_daemon);
    TEST_CHECK_FALSE(b.get_is_daemon());
    // syslog level
    TEST_CHECK_TRUE(b.get_settings()->use_syslog == 5);
    TEST_CHECK_TRUE(b.get_syslog_level() == 5);
    // addr
    TEST_CHECK_TRUE(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_family == 2);
    TEST_CHECK_TRUE(be16toh(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_port) == 8888);
    TEST_CHECK_TRUE(((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_addr.s_addr == 0x02020202U);
    TEST_CHECK_TRUE(b.tst_local_base_as_inet().sin_family == 2);
    TEST_CHECK_TRUE(be16toh(b.tst_local_base_as_inet().sin_port) == 8888);
    TEST_CHECK_TRUE(b.tst_local_base_as_inet().sin_addr.s_addr == 0x02020202U);
    TEST_CHECK_TRUE(b.get_local_base_str() == "2.2.2.2:8888");
    TEST_CHECK_TRUE(& ((struct sockaddr_in *) b.get_settings()->local_base_.data())->sin_addr.s_addr == & b.tst_local_base_as_inet().sin_addr.s_addr);
    // sys lib dir
    TEST_CHECK_TRUE(b.get_settings()->lib_dir == "/bubin/libs");
    TEST_CHECK_TRUE(b.get_lib_dir() == "/bubin/libs/");
    TEST_CHECK_TRUE(b.get_lib_dir().data() != b.get_settings()->lib_dir.data());
    // fb lib name
    TEST_CHECK_TRUE(b.get_settings()->lib_name == "ib_client");
    TEST_CHECK_TRUE(b.get_lib_name_fb() == "ib_client");
    TEST_CHECK_TRUE(b.get_lib_name_fb().data() != b.get_settings()->lib_name.data());
    // root dir
    TEST_CHECK_TRUE(b.get_settings()->root_dir == "/tmp/qwerty");
    TEST_CHECK_TRUE(b.get_root_dir() == "/tmp/qwerty/");
    TEST_CHECK_TRUE(b.get_root_dir().data() != b.get_settings()->root_dir.data());
    // search dirs
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs.size() == 2);
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs[0] == "/var/ququ/dir1");
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs[1] == "/var/ququ/dir2");
    b.set_search_dir();
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs.size() == 0);
    b.set_search_dir_append("/root/d1");
    b.set_search_dir_append("/root/d2");
    TEST_CHECK_TRUE(b.get_settings()->backup_dirs.size() == 2);
    // fb account
    TEST_CHECK_TRUE(b.get_settings()->db == "qq.gdb");
    TEST_CHECK_TRUE(b.get_settings()->user == "admin");
    TEST_CHECK_TRUE(b.get_settings()->pass == "passss");
    TEST_CHECK_TRUE(b.get_settings()->role == "no_role");
    TEST_CHECK_TRUE(b.get_settings()->dialect == 2);
    {
      auto [b1,u1,p1,r1,d1] = b.get_connection();
      TEST_CHECK_TRUE(b1 == "qq.gdb");
      TEST_CHECK_TRUE(u1 == "admin");
      TEST_CHECK_TRUE(p1 == "passss");
      TEST_CHECK_TRUE(r1 == "no_role");
      TEST_CHECK_TRUE(d1 == 2);
      TEST_CHECK_TRUE(b1.data() != b.get_settings()->db.data());
      TEST_CHECK_TRUE(u1.data() != b.get_settings()->user.data());
      TEST_CHECK_TRUE(p1.data() != b.get_settings()->pass.data());
      TEST_CHECK_TRUE(r1.data() != b.get_settings()->role.data());
    }

    // 4
    START_ITER("set options - IP address parsing");

    b.set_local_base("5.5.5.5");
    TEST_CHECK_TRUE(b.get_local_base_str() == "5.5.5.5:69");

    b.set_local_base("1.2.3.4:");
    TEST_CHECK_TRUE(b.get_local_base_str() == "1.2.3.4:69");

    b.set_local_base("1.2.3.4:12345");
    TEST_CHECK_TRUE(b.get_local_base_str() == "1.2.3.4:12345");

    b.set_local_base(":11");
    TEST_CHECK_TRUE(b.get_local_base_str() == "0.0.0.0:11");

    b.set_local_base(":22222");
    TEST_CHECK_TRUE(b.get_local_base_str() == "0.0.0.0:22222");

    b.set_local_base("fe80::c259:adaf:265c:8262:69");
    TEST_CHECK_TRUE(b.get_local_base_str() == "[fe80::c259:adaf:265c:8262]:69");

    b.set_local_base("[fe80::c259:adaf:265c:8262]:69");
    TEST_CHECK_TRUE(b.get_local_base_str() == "[fe80::c259:adaf:265c:8262]:69");

    b.set_local_base("fe80::8262:69");
    TEST_CHECK_TRUE(b.get_local_base_str() == "[fe80::8262]:69");

    b.set_local_base("[fe80::8262]:69");
    TEST_CHECK_TRUE(b.get_local_base_str() == "[fe80::8262]:69");

    b.set_local_base("[::]:69");
    TEST_CHECK_TRUE(b.get_local_base_str() == "[::]:69");

    b.set_local_base("[::1]:690");
    TEST_CHECK_TRUE(b.get_local_base_str() == "[::1]:690");
  }

  //
UNIT_TEST_CASE_END

//---------------------------------------------------------------------------------------------------------------------------------

UNIT_TEST_SUITE_END
