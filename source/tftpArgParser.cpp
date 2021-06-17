/**
 * \file tftpArgParser.cpp
 * \brief Class ArgParser module
 *
 *  Class for CMD arguments parsing
 *
 *  License GPL-3.0
 *
 *  \date 10-jun-2021
 *  \author Vitaliy Shirinkin, e-mail: vitaliy.shirinkin@gmail.com
 *
 *  \version 0.2
 */

#include <sstream>

#include "tftpArgParser.h"

namespace tftp
{

// -----------------------------------------------------------------------------

auto ArgParser::check_arg(const char * ptr_str)
    -> std::tuple<ArgParser::ArgType, std::string>
{
  if(ptr_str == nullptr) return {ArgType::not_found, ""};

  std::string tmp_str{ptr_str};

  if(tmp_str.size() < 2U) return {ArgType::normal_value, tmp_str};

  if(tmp_str[0U] != '-') return {ArgType::normal_value, tmp_str};

  if(tmp_str[1U] != '-') return {ArgType::is_short, std::string{tmp_str.cbegin()+1, tmp_str.cend()}};

  if(tmp_str.size() == 2U) return {ArgType::end_parse, ""};

  if(tmp_str[2U] != '-') return {ArgType::is_long, std::string{tmp_str.cbegin()+2, tmp_str.cend()}};

  return {ArgType::normal_value, tmp_str};
}

// -----------------------------------------------------------------------------

auto ArgParser::get_line_out(const ArgItem & item) -> std::string
{
  std::string ret;

  auto & names = std::get<VecStr>(item);
  auto & type_arg = std::get<ArgExistVaue>(item);
  auto & val_capt = std::get<3>(item);
  auto & caption = std::get<4>(item);

  ret = construct_args(names);

  if(ret.size() && (type_arg != ArgExistVaue::no))
  {
    ret.append(" ");

    if(type_arg == ArgExistVaue::optional) ret.append("[");

    if((type_arg == ArgExistVaue::optional) ||
       (type_arg == ArgExistVaue::required))
    {
      ret.append("<");
      if(val_capt.size()) ret.append(val_capt); else ret.append("value");
      ret.append(">");
    }

    if(type_arg == ArgExistVaue::optional) ret.append("]");
  }

  if(ret.size()) ret.append(" ");

  if(caption.size() > 0U) ret.append(caption);
                     else  ret.append("...");

  return ret;
}

// -----------------------------------------------------------------------------

auto ArgParser::go(
    const ArgItems & items_ref,
    int argc,
    char * argv[]) -> ArgRes
{
  ArgRes ret;
  bool is_end_parse=false;

  for(int iter=1; iter < argc; ++iter)
  {
    if(is_end_parse)
    {
      ret.second.emplace_back(argv[iter]);
      continue;
    }

    auto [a_type, a_value] = check_arg(argv[iter]);

    if(a_type == ArgType::end_parse) { is_end_parse=true; continue; }

    if(a_type == ArgType::not_found) break; // error input data (argv)

    if((a_type == ArgType::normal_value))
    {
      ret.second.push_back(a_value);
      continue;
    }

    bool need_skip_next=false;

    // Get search list (for correct pass short multi options)
    VecStr a_vals;
    if(a_type == ArgType::is_long) a_vals.push_back(a_value);
    if(a_type == ArgType::is_short)
    {
      for(auto & chr : a_value) a_vals.push_back(std::string{chr});
    }

    // Check next argument
    ArgType     next_type{ArgType::not_found};
    std::string next_value;
    if((iter+1) < argc)
    {
      std::tie(next_type, next_value) = check_arg(argv[iter+1]);
    }

    // Search!
    for(const auto & itm : items_ref)
    {
      const auto & itm_type_val = std::get<ArgExistVaue>(itm);

      for(const auto & chk_name : std::get<VecStr>(itm))
      {
        if((a_type == ArgType::is_long)  && (chk_name.size() < 2U)) continue;
        if((a_type == ArgType::is_short) && (chk_name.size() != 1U)) continue;

        for(auto & passed_name : a_vals)
        {
          if(chk_name == passed_name) // if find option
          {
            auto & res_list = ret.first[std::get<int>(itm)]; // create if need

            if((next_type == ArgType::normal_value) &&
               ((itm_type_val == ArgExistVaue::required) ||
                (itm_type_val == ArgExistVaue::optional)))
            {
              res_list.emplace_back(ArgParser::construct_arg(passed_name), next_value);
              need_skip_next = true;
            }
            else
            {
              res_list.emplace_back(ArgParser::construct_arg(passed_name), "");
            }

            break;
          }
        }
      }
    }

    if(need_skip_next) ++iter;
  }

  return ret;
}

// -----------------------------------------------------------------------------

auto ArgParser::construct_arg(const std::string & arg_name) -> std::string
{
  if(arg_name.size() == 0U) return "";

  if((arg_name[0U] == '-') ||
     (arg_name[0U] == ' ')) return "";

  std::string ret{"-"};
  if(arg_name.size() > 1U) ret.append("-");
  ret.append(arg_name);

  return ret;
}

// -----------------------------------------------------------------------------

auto ArgParser::construct_args(const VecStr & arg_names) -> std::string
{

  std::stringstream ss;
  size_t was_printed = 0U;

  for(auto & name : arg_names)
  {
    std::string tmp_str{construct_arg(name)};

    if(tmp_str.size() == 0U) continue;

    if(was_printed) ss << "|";
    ss << tmp_str;
    ++was_printed;
  }

  if(was_printed > 1U) return "{"+ss.str()+"}";

  return ss.str();
}

// -----------------------------------------------------------------------------

void ArgParser::out_help_data(
    const ArgItems & items,
    std::ostream & stream,
    std::string_view app_name)
{
  if(items.size()==0U)
  {
    stream << "No any possible options!" << std::endl;
  }
  else
  {
    if(std::get<VecStr>(items[0U]).size() != 0U)
    {
      stream << "Usage:" << std::endl;
      stream << (app_name.size() ?  app_name : "./<app_name>");
      stream << " [opt1> [<opt1 value>]] ... [<main argument 1>] ..." << std::endl;
      stream << "Possible options:" << std::endl;
    }

    for(auto & item : items) stream << get_line_out(item) << std::endl;
  }
}

// -----------------------------------------------------------------------------

} // namespace tftp
