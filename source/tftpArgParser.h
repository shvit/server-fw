/*
 * tftpArgParser.h
 *
 *  Created on: 10 июн. 2021 г.
 *      Author: svv
 */

#ifndef SOURCE_TFTPARGPARSER_H_
#define SOURCE_TFTPARGPARSER_H_

#include <array>
#include <string>
#include <tuple>
#include <functional>
#include <vector>
#include <map>

namespace tftp
{

// -----------------------------------------------------------------------------

using VecStr = std::vector<std::string>;

enum class ArgExistVaue: int
{
  no = 0,
  required = 1,
  optional = 2,
};

using ArgItem = std::tuple<
    int,
    VecStr,
    ArgExistVaue,
    std::string>;

using ArgItems = std::vector<ArgItem>;

//using fParseItem = std::function<void(const int &, std::string)>;

using ArgResItemPair = std::pair<std::string, std::string>;
using ArgResItemPairs = std::vector<ArgResItemPair>;
using ArgResItems = std::map<int, ArgResItemPairs>;
using ArgRes = std::pair<ArgResItems, VecStr>;

// -----------------------------------------------------------------------------

class ArgParser
{
protected:

  enum class ArgType: int
  {
    normal_value,
    is_short,
    is_long,
    not_found,
  };

  auto check_arg(const char * ptr_str) const
      -> std::tuple<ArgType, std::string>;

public:

  //auto go(
  //    const ArgItems & items_ref,
  //    fParseItem cb,
  //    int argc,
  //    char * argv[]) -> VecStr;

  auto go_full(
      const ArgItems & items_ref,
      int argc,
      char * argv[]) const -> ArgRes;

  void out_help_data(
      const ArgItems & items,
      std::ostream & stream,
      std::string_view app_name) const;

  auto construct_arg(const std::string & arg_name) const -> std::string;

};

// -----------------------------------------------------------------------------


} // namespace tftp

#endif /* SOURCE_TFTPARGPARSER_H_ */
