#ifndef _H_COMMON_UTILS_H_
#define _H_COMMON_UTILS_H_

#include <string>
#include <vector>
#include <windows.h>
#include <direct.h>
#include <io.h>

// if the file exists
BOOL is_file_exist(LPCTSTR csFile);

// if the directory exists
BOOL is_dir_exist(LPCTSTR csDir);

// if the file or directory exists
BOOL is_path_exist(LPCTSTR csPath);

// create the directory recursively
void mkdir_recursively(const std::string& strPath);

/**
* @brief split the string
* @param str -- the string trimed
* @param seperator -- the seperator string
* @return the splited string list
*/
std::vector<std::string> string_split(const std::string& str, const std::string& seperator);

/**
* @brief if the string starts with substr
* @param str -- the string
* @param substr -- the substr
* @return
*/
bool string_starts_with(const std::string& str, const std::string& substr);

/**
* @brief if the string ends with substr
* @param str -- the string
* @param substr -- the substr
* @return
*/
bool string_ends_with(const std::string& str, const std::string& substr);


/**
* @brief trim char in trimChars from end of str
* @param str -- the string trimed
*/
void string_trim_end(std::string& str);

/**
* @brief trim char in trimChars from start of str
* @param str -- the string trimed
*/
void string_trim_start(std::string& str);

/**
* @brief trim char in trimChars from start and end of str
* @param str -- the string trimed
*/
void string_trim(std::string& str);

/**
* @brief convert chars in string to lower
* @param str -- the string
*/
void string_to_lower(std::string& str);

/**
* @brief convert chars in string to upper
* @param str -- the string
*/
void string_to_upper(std::string& str);

/**
* @brief replace substring 'search' in string with 'replacement'
* @param str -- the string
*        search -- the substring be replaced
*        replacement -- replace string
*/
void string_replace(std::string& str, const std::string& search, const std::string& replacement);

#endif