/*
 * tftpArgParser.cpp
 *
 *  Created on: 10 июн. 2021 г.
 *      Author: svv
 */

#include <iostream>

#include "tftpArgParser.h"

namespace tftp
{

// -----------------------------------------------------------------------------

auto ArgParser::check_arg(const char * ptr_str) const
    -> std::tuple<ArgParser::ArgType, std::string>
{
  if(ptr_str == nullptr) return {ArgType::not_found, ""};

  std::string tmp_str{ptr_str};

  if(tmp_str.size() > 2U)
  {
    if((tmp_str[0U] == '-') &&
       (tmp_str[1U] == '-') &&
       (tmp_str[2U] != '-'))
    {
      return {ArgType::is_long, std::string{tmp_str.cbegin()+2, tmp_str.cend()}};
    }
  }

  if(tmp_str.size() > 1U)
  {
    if((tmp_str[0U] == '-') &&
       (tmp_str[1U] != '-'))
    {
      return {ArgType::is_short, std::string{tmp_str.cbegin()+1, tmp_str.cend()}};
    }
  }

  return {ArgType::normal_value, tmp_str};
}

// -----------------------------------------------------------------------------
/*
auto ArgParser::go(
    const ArgItems & items_ref,
    fParseItem cb,
    int argc,
    char * argv[]) -> VecStr
{
  //std::cout << " @ argc=" << argc << std::endl;
  VecStr ret;

  for(int iter=1; iter < argc; ++iter)
  {
    //std::cout << " @@ arguments iter " << iter << std::endl;
    auto [a_type, a_value] = check_arg(argv[iter]);

    if(a_type == ArgType::not_found) break; // error input data (argv)

    if((a_type == ArgType::normal_value))
    {
      ret.push_back(a_value);
      continue;
    }

    //std::cout << " @@ items_ref.size()=" << items_ref.size() << std::endl;

    for(size_t it_list=0U; it_list < items_ref.size(); ++it_list)
    {
      //std::cout << " @@@ search iter(list) " << it_list << std::endl;
      const auto & names = std::get<VecStr>(items_ref[it_list]);

      for(size_t it_name=0U; it_name < names.size(); ++it_name)
      {
        //std::cout << " @@@   search iter(name) " << it_name << std::endl;
        if((a_type == ArgType::is_long) 232334345 && (names[it_name].size() < 2U)) continue;
        if((a_type == ArgType::is_short) && (names[it_name].size() != 1U)) continue;

        if(names[it_name] == a_value) // if find option
        {
          if((iter+1) < argc) // this item is not end of list
          {
            auto [an_type, an_value] = check_arg(argv[iter+1]);

            if((an_type == ArgType::normal_value) &&
                ((std::get<ArgExistVaue>(items_ref[it_list]) == ArgExistVaue::required) ||
                 (std::get<ArgExistVaue>(items_ref[it_list]) == ArgExistVaue::optional)))
            {
              cb(std::get<int>(items_ref[it_list]), an_value);
              ++iter;
              break;
            }
            else
            {
              cb(std::get<int>(items_ref[it_list]), "");
              break;
            }
          }
          else
          {
            cb(std::get<int>(items_ref[it_list]), "");
            break;
          }
        }
      }
    }
  }

  return ret;
}
*/
// -----------------------------------------------------------------------------

auto ArgParser::go_full(
    const ArgItems & items_ref,
    int argc,
    char * argv[]) const -> ArgRes
{
  //std::cout << " @ argc=" << argc << std::endl;
  ArgRes ret;

  for(int iter=1; iter < argc; ++iter)
  {
    //std::cout << " @@ arguments iter " << iter << std::endl;
    auto [a_type, a_value] = check_arg(argv[iter]);

    if(a_type == ArgType::not_found) break; // error input data (argv)

    if((a_type == ArgType::normal_value))
    {
      ret.second.push_back(a_value);
      continue;
    }

    //std::cout << " @@ items_ref.size()=" << items_ref.size() << std::endl;

    for(size_t it_list=0U; it_list < items_ref.size(); ++it_list)
    {
      //std::cout << " @@@ search iter(list) " << it_list << std::endl;
      const auto & names = std::get<VecStr>(items_ref[it_list]);

      for(size_t it_name=0U; it_name < names.size(); ++it_name)
      {
        //std::cout << " @@@   search iter(name) " << it_name << std::endl;
        if((a_type == ArgType::is_long)  && (names[it_name].size() < 2U)) continue;
        if((a_type == ArgType::is_short) && (names[it_name].size() != 1U)) continue;

        if(names[it_name] == a_value) // if find option
        {
          //auto & id =
          auto & res_list = ret.first[std::get<int>(items_ref[it_list])];

          if((iter+1) < argc) // this item is not end of list
          {
            auto [an_type, an_value] = check_arg(argv[iter+1]);

            if((an_type == ArgType::normal_value) &&
                ((std::get<ArgExistVaue>(items_ref[it_list]) == ArgExistVaue::required) ||
                 (std::get<ArgExistVaue>(items_ref[it_list]) == ArgExistVaue::optional)))
            {
              //cb(std::get<int>(items_ref[it_list]), an_value);
              res_list.emplace_back(argv[iter], an_value);
              ++iter;
              break;
            }
            else
            {
              //cb(std::get<int>(items_ref[it_list]), "");
              res_list.emplace_back(argv[iter], "");
              break;
            }
          }
          else
          {
            //cb(std::get<int>(items_ref[it_list]), "");
            res_list.emplace_back(argv[iter], "");
            break;
          }
        }
      }
    }
  }

  return ret;
}

// -----------------------------------------------------------------------------

auto ArgParser::construct_arg(const std::string & arg_name) const -> std::string
{
  std::string ret{"-"};
  if(arg_name.size() > 1U) ret.append("-");
  ret.append(arg_name);

  return ret;
}

// -----------------------------------------------------------------------------

void ArgParser::out_help_data(
    const ArgItems & items,
    std::ostream & stream,
    std::string_view app_name) const
{
  stream << "Usage:" << std::endl << app_name;
  if(items.size()==0U)
  {
    stream << std::endl;
    stream << "No any possible options!" << std::endl;
  }
  else
  {
    stream << " [<opt1> [<opt1 value>]] [<opt2> [<opt2 value>]] ... [value1] [value2] ..." << std::endl;
    stream << "Possible options:" << std::endl;

    for(auto & item : items)
    {
      auto & names = std::get<VecStr>(item);
      auto & type_arg = std::get<ArgExistVaue>(item);
      auto & caption = std::get<std::string>(item);
      auto cnt_names=names.size();

      if(cnt_names > 0U)
      {
        if(cnt_names > 1U) stream << "{";
        bool was_printed=false;
        for(auto & name : names)
        {
          if(was_printed) stream << "|";
          stream << construct_arg(name);
          was_printed=true;
        }
        if(cnt_names > 1U) stream << "}";

        if(type_arg != ArgExistVaue::no) stream << " ";

        if(type_arg == ArgExistVaue::optional) stream << "[";

        if((type_arg == ArgExistVaue::optional) ||
           (type_arg == ArgExistVaue::required)) stream << "<value>";

        if(type_arg == ArgExistVaue::optional) stream << "]";
      }

      if(caption.size() > 0U)
      {
        if(cnt_names > 0U) stream << " ";
        stream << caption << std::endl;
      }
    }
  }
}

// -----------------------------------------------------------------------------

} // namespace tftp
