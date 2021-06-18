/**
 * \file tftpArgParser.h
 * \brief Class ArgParser header
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

#ifndef SOURCE_TFTPARGPARSER_H_
#define SOURCE_TFTPARGPARSER_H_

#include <map>
#include <string>
#include <tuple>
#include <vector>

namespace tftp
{

// -----------------------------------------------------------------------------

/** \brief Vector of strings
 */
using VecStr = std::vector<std::string>;

/** \brief Enum type with argument exist flag
 */
enum class ArgExistVaue: int
{
  no = 0,       ///< No any values
  required = 1, ///< Need one value
  optional = 2, ///< Value can or not can be
};

/** \brief Setting for options with one ID
 *
 *  Tuple index:
 *  0: any unique value - ID
 *  1: Vector of option names with one ID; one char - short, more chars - long
 *  2: Attribute with exists value or not
 *  3: Caption of value for option
 *  4: Common caption for option
 *  5: Additional info for caption in round bracket
 */
using ArgItem = std::tuple<
    int,          ///< ID for unique actions - use at top level switch
    VecStr,       ///< Argument option names vector
    ArgExistVaue, ///< Attribute of value for argument
    std::string,  ///< Name of value for argument
    std::string,  ///< Caption of argument - full description
    std::string>; ///< Additional info for caption in round bracket (...)

using ArgItems = std::vector<ArgItem>;

// -----------------------------------------------------------------------------

/** \brief Type for argument option (first) and its value (second)
 */
using ArgResItemPair = std::pair<std::string, std::string>;

/** \brief Vector of grouped by ID for arguments and its options
 */
using ArgResItemPairs = std::vector<ArgResItemPair>;

/** \brief Map with key as ID and values with parsed options
 */
using ArgResItems = std::map<int, ArgResItemPairs>;

/** \brief Full result of parsing - pair of parsed and unparsed values
 *
 *  first - succesfully parsed argument options
 *  second - unparsed CMD arguments
 */
using ArgRes = std::pair<ArgResItems, VecStr>;

enum class ResCheck: int
{
  not_found,
  normal,
  wrn_many_arg,
  err_no_req_value,
  err_wrong_data,
};

// -----------------------------------------------------------------------------

/** \brief Parser of command line argumants
 *
 *  Source:
 *  - settings arguments values as type ArgItems
 *  - standart argc,argv
 *
 *  Main method is go()
 *
 *  Support multi short options:
 *  "-Fuck" == "-F -u -c -k"
 *  "-aX value" == "-a value -X value" ('a' and 'x' is reuired/optional)
 */
class ArgParser
{
protected:
  ArgItems data_settings_;
  ArgRes   data_result_;


  enum class ArgType: int
  {
    normal_value,
    is_short,
    is_long,
    end_parse,
    not_found,
  };

  /** \brief Check char pointer is argument option
   *
   *  For short arguments need "-" at start position and one or more chars
   *  For long arguments need "--" at start position and one or more chars
   *  If find only "--" then need end partse and return ArgType::end_parse
   *  \param [in] ptr_str Source pointer to chars
   *  \return Tuple with type ArgType and result string (without -/--)
   */
  auto chk_arg(const char * ptr_str) const
      -> std::tuple<ArgType, std::string>;

  /** \brief Construct string with right argument option
   *
   *  Used for help output
   *  Add -/-- at start of string argument name:
   *  - if legth=1 then add "-" at begin
   *  - if length>1 then add "--" at begin
   *  If string name is wrong then return ""
   *  \param [in] arg_name Argument name
   *  \return String with right option
   */
  auto constr_arg(const std::string & arg_name) const -> std::string;

  /** \brief Construct right argument all options
   *
   *  Used for help output
   *  Use construct_arg() for options
   *  If options count >1 then add: '{', '|', '}'
   *  \param [in] arg_names Vector with names
   *  \return Result string
   */
  auto constr_args(const VecStr & arg_names) -> std::string;

  /** \brief Out one line with argument option data
   *
   *  Used for help output
   *  Use construct_args() for options
   *  If no option names then out only captions
   *  \param [in] item Tuple of one argument option
   *  \return Result one string line
   */
  auto constr_line_out(const ArgItem & item) -> std::string;





  /** \brief Check char pointer is argument option
   *
   *  For short arguments need "-" at start position and one or more chars
   *  For long arguments need "--" at start position and one or more chars
   *  If find only "--" then need end partse and return ArgType::end_parse
   *  \param [in] ptr_str Source pointer to chars
   *  \return Tuple with type ArgType and result string (without -/--)
   */
  static auto check_arg(const char * ptr_str)
      -> std::tuple<ArgType, std::string>;

  /** \brief Construct string with right argument option
   *
   *  Used for help output
   *  Add -/-- at start of string argument name:
   *  - if legth=1 then add "-" at begin
   *  - if length>1 then add "--" at begin
   *  If string name is wrong then return ""
   *  \param [in] arg_name Argument name
   *  \return String with right option
   */
  static auto construct_arg(const std::string & arg_name) -> std::string;

  /** \brief Construct right argument all options
   *
   *  Used for help output
   *  Use construct_arg() for options
   *  If options count >1 then add: '{', '|', '}'
   *  \param [in] arg_names Vector with names
   *  \return Result string
   */
  static auto construct_args(const VecStr & arg_names) -> std::string;

  /** \brief Out one line with argument option data
   *
   *  Used for help output
   *  Use construct_args() for options
   *  If no option names then out only captions
   *  \param [in] item Tuple of one argument option
   *  \return Result one string line
   */
  static auto get_line_out(const ArgItem & item) -> std::string;

public:

  ArgParser();

  ArgParser(const ArgItems & new_val);

  /** \brief Parsing argument options
   *
   *  \param [in] items_ref Source settings for all arguments options
   *  \param [in] argc Count of arguments from CMD
   *  \param [in] argv Arguments from CMD
   *  \return Parsed data as type ArgRes
   */
  auto run(
      int argc,
      char * argv[]) -> const ArgRes &;

  auto constr_caption(const int & id) -> std::string;






  /** \brief Parsing argument options
   *
   *  \param [in] items_ref Source settings for all arguments options
   *  \param [in] argc Count of arguments from CMD
   *  \param [in] argv Arguments from CMD
   *  \return Parsed data as type ArgRes
   */
  static auto go(
      const ArgItems & items_ref,
      int argc,
      char * argv[]) -> ArgRes;

  /** \brief Out to stream help information of arguments options
   *
   *  Used for help output
   *  \param [in] items Source settings for all arguments options
   *  \param [out] stream Output stream
   *  \param [in] Application name (filename); default is ""
   */
  static void out_help_data(
      const ArgItems & items,
      std::ostream & stream,
      std::string_view app_name="");

  static auto construct_caption(
      const ArgItems & items,
      const int & id) -> std::string;

  // Parsing results

  static auto chk_result(
      const ArgItems & items,
      const int & id,
      const ArgResItems & res) -> std::tuple<ResCheck,std::string>;

  static auto get_last_value(
      const ArgResItems & res,
      const int & id) -> std::string;

};

// -----------------------------------------------------------------------------

} // namespace tftp

#endif /* SOURCE_TFTPARGPARSER_H_ */
