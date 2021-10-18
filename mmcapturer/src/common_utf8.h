#ifndef _H_COMMON_UTF8_H_
#define _H_COMMON_UTF8_H_

#include <string>
#include <stdint.h>

/**
* @brief check if the string is utf8 string
* @param str - the character array
*
* @return true - utf8 string 
* @return false -- other strings
*/
bool is_utf8_chars(const uint8_t *str);

/**
* @brief check if the string is utf8 string
* @param str - the string
*
* @return true - utf8 string
* @return false -- other strings
*/
bool is_utf8_str(const std::string& str);

/**
* @brief convert gbk string to utf8 string
* @param str - the gbk string
*
* @return utf8 string
*/
std::string gbk_to_utf8(const std::string& str);

/**
* @brief convert utf8 string to gbk string
* @param str - the utf8 string
*
* @return gbk string
*/
std::string utf8_to_gbk(const std::string& str);

/**
* @brief convert utf8 string to wchar string
* @param in -- the utf8 input string
*        in_size -- the utf8 length, if it is 0, then the length is automatically computed inside this function
*        out -- the wchar_t output buffer
*        out_size -- the wchar_t output buffer size
* @return the number of characters written to the buffer
*/
size_t utf8_to_wchar(const char *in, size_t in_size, wchar_t *out,
	size_t out_size);

/**
* @brief convert wchar_t string to utf8 string
* @param in -- the wchar_t input string
*        in_size -- the wchar_t length, if it is 0, then the length is automatically computed inside this function
*        out -- the utf8 output buffer
*        out_size -- the utf8 output buffer size
* @return the number of bytes written to the buffer
*/
size_t wchar_to_utf8(const wchar_t *in, size_t in_size, char *out,
	size_t out_size);
#endif