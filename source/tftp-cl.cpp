/*
 * tftp-cl.cpp
 *
 *  Created on: 23 июн. 2021 г.
 *      Author: svv
 */

#include <future>
#include <iostream>
#include <string>

#include "tftpClientSettings.h"
#include "tftpClientSession.h"

int main(int argc, char* argv[])
{
  tftp::ClientSession s(&std::cout);

  auto f_run = std::async(
      std::bind(
          & tftp::ClientSession::run,
          & s,
          argc,
          argv));

  auto ret = f_run.get();

  if(ret != tftp::ClientSessionResult::ok) return EXIT_FAILURE;

  return EXIT_SUCCESS;
}
