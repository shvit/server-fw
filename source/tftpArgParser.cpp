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

ArgParser::ArgParser():
    data_settings_{},
    data_result_{}
{
}

// -----------------------------------------------------------------------------

ArgParser::ArgParser(const ArgItems & new_sett):
    data_settings_{new_sett},
    data_result_{}
{
}

// -----------------------------------------------------------------------------

auto ArgParser::run(
    int argc,
    char * argv[]) -> const ArgRes &
{
  data_result_.first.clear();
  data_result_.second.clear();

  bool is_end_parse=false;

  for(int iter=1; iter < argc; ++iter)
  {
    if(is_end_parse)
    {
      data_result_.second.emplace_back(argv[iter]);
      continue;
    }

    auto [a_type, a_value] = chk_arg(argv[iter]);

    if(a_type == ArgType::end_parse) { is_end_parse=true; continue; }

    if(a_type == ArgType::not_found) break; // error input data (argv)

    if((a_type == ArgType::normal_value))
    {
      data_result_.second.push_back(a_value);
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
      std::tie(next_type, next_value) = chk_arg(argv[iter+1]);
    }

    // Search!
    for(const auto & itm : data_settings_)
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
            auto & res_list = data_result_.first[std::get<int>(itm)]; // create if need

            if((next_type == ArgType::normal_value) &&
               ((itm_type_val == ArgExistVaue::required) ||
                (itm_type_val == ArgExistVaue::optional)))
            {
              res_list.emplace_back(constr_arg(passed_name), next_value);
              need_skip_next = true;
            }
            else
            {
              res_list.emplace_back(constr_arg(passed_name), "");
            }

            break;
          }
        }
      }
    }

    if(need_skip_next) ++iter;
  }

  return data_result_;
}

// -----------------------------------------------------------------------------

auto ArgParser::chk_arg(const char * ptr_str) const
    -> std::tuple<ArgType, std::string>
{
  if(ptr_str == nullptr) return {ArgType::not_found, ""};

  std::string tmp_str{ptr_str};

  if(tmp_str.size() < 2U) return {ArgType::normal_value, tmp_str};

  if(tmp_str[0U] != '-') return {ArgType::normal_value, tmp_str};

  if(tmp_str[1U] != '-') return {ArgType::is_short, std::string{tmp_str.cbegin()+1, tmp_str.cend()}};

  if(tmp_str.size() == 2U) return {ArgType::end_parse, "--"};

  if(tmp_str[2U] != '-') return {ArgType::is_long, std::string{tmp_str.cbegin()+2, tmp_str.cend()}};

  return {ArgType::normal_value, tmp_str};
}

// -----------------------------------------------------------------------------

auto ArgParser::constr_arg(const std::string & arg_name) const -> std::string
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

auto ArgParser::constr_args(const VecStr & arg_names) const -> std::string
{
  std::stringstream ss;
  size_t was_printed = 0U;

  for(auto & name : arg_names)
  {
    std::string tmp_str{constr_arg(name)};

    if(tmp_str.size() == 0U) continue;

    if(was_printed) ss << "|";
    ss << tmp_str;
    ++was_printed;
  }

  if(was_printed > 1U) return "{"+ss.str()+"}";

  return ss.str();
}

// -----------------------------------------------------------------------------

auto ArgParser::constr_caption(const int & id) const -> std::string
{
  std::string ret;

  bool data_ok = false;
  size_t src_idx;
  for(size_t it=0U;  it < data_settings_.size(); ++it)
  {
    if(std::get<int>(data_settings_[it]) == id)
    {
      data_ok = true;
      src_idx = it;
      break;
    }
  }
  if(data_ok)
  {
    ret = std::get<4>(data_settings_[src_idx]);
    if(ret.size() == 0U) ret = "Action #" + std::to_string(id);
    if(ret == "--") ret.clear();
  }

  return ret;
}

// -----------------------------------------------------------------------------

auto ArgParser::constr_line_out(const ArgItem & item) const -> std::string
{
  std::string ret;

  auto & names = std::get<VecStr>(item);
  auto & type_arg = std::get<ArgExistVaue>(item);
  auto & val_capt = std::get<3>(item);
  auto & caption = std::get<4>(item);
  auto & note = std::get<5>(item);

  ret = constr_args(names);

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

  // Add () Caption
  if(ret.size()) ret.append(" ");
  ret.append(caption);
  if(ret.size() && (caption.size() == 0U)) ret.append("...");

  // Add () Note
  if(note.size())
  {
    if(ret.size()) ret.append(" ");
    ret.append("(");
    ret.append(note);
    ret.append(")");
  }

  return ret;
}

// -----------------------------------------------------------------------------

void ArgParser::out_help(
    std::ostream & stream,
    std::string_view app_name) const
{
  if(data_settings_.size()==0U)
  {
    stream << "No any possible options!" << std::endl;
  }
  else
  {
    if(std::get<VecStr>(data_settings_[0U]).size() != 0U)
    {
      stream << "Usage:" << std::endl;
      stream << (app_name.size() ?  app_name : "./<app_name>");
      stream << " [opt1> [<opt1 value>]] ... [<main argument 1>] ..." << std::endl;
      stream << "Possible options:" << std::endl;
    }

    for(auto & item : data_settings_)
    {
      std::string tmp_str = constr_line_out(item);
      if(tmp_str == "--") continue;
      stream << tmp_str << std::endl;
    }
  }
}

// -----------------------------------------------------------------------------

void ArgParser::out_header(std::ostream & stream) const
{
  for(auto & item : data_settings_)
  {
    if(std::get<VecStr>(item).size() > 0U) break;
    if(std::get<4>(item) == "--") break;

    stream << constr_line_out(item) << std::endl;
  }
}

// -----------------------------------------------------------------------------

auto ArgParser::chk_parsed_item(const int & id) const
    -> std::tuple<ResCheck, std::string>
{
  // Find source data exist by ID
  bool data_ok=false;
  size_t src_idx;
  for(size_t it=0U;  it < data_settings_.size(); ++it)
  {
    if(std::get<int>(data_settings_[it]) == id)
    {
      data_ok = true;
      src_idx = it;
      break;
    }
  }
  if(!data_ok)
  {
    return {ResCheck::err_wrong_data,
            "Error not found source data for action id #"+std::to_string(id)};
  }

  // Find ID in result
  auto curr_res = data_result_.first.find(id);
  if(curr_res == data_result_.first.end())
  {
    return {ResCheck::not_found,
            "No argument for '"+constr_caption(id)+"'"};
  }

  // Check if required not exist
  auto cnt_res = curr_res->second.size();
  if((std::get<ArgExistVaue>(data_settings_[src_idx]) == ArgExistVaue::required) &&
      (cnt_res == 0U))
  {
    return {ResCheck::err_no_req_value,
            "No required argument '"+constr_caption(id)+"'"};
  }

  // Check many count args
  if(cnt_res > 1U)
  {
    return {ResCheck::wrn_many_arg,
            "Many arguments for '"+constr_caption(id)+"'"};
  }

  return {ResCheck::normal, ""};
}

// -----------------------------------------------------------------------------

auto ArgParser::get_parsed_item(const int & id) const -> std::string
{
  auto it = data_result_.first.find(id);

  if(it == data_result_.first.end()) return "";

  if(it->second.size() == 0U) return "";

  return it->second[it->second.size() -1U].second;
}

// -----------------------------------------------------------------------------

auto ArgParser::get_parsed_items(const int & id) const -> VecStr
{
  auto it = data_result_.first.find(id);

  VecStr tmp_list;

  if(it == data_result_.first.end()) return tmp_list;

  if(it->second.size() == 0U) return tmp_list;

  for(auto & item : it->second)
  {
    tmp_list.push_back(item.second);
  }

  return tmp_list;

}

// -----------------------------------------------------------------------------

auto ArgParser::get_parsed_int(const int & id) const -> std::optional<int>
{
  std::optional<int> ret;

  std::string tmp_str = get_parsed_item(id);
  if(tmp_str.size())
  {
    size_t pos;

    try
    {
      int tmp_int = std::stoi(tmp_str, & pos, 10);
      ret = tmp_int;
    }
    catch(...) {}
  }

  return ret;
}

// -----------------------------------------------------------------------------

} // namespace tftp
