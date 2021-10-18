#include "common_utils.h"

#include <algorithm>
#include <cctype>

// if the file exists
BOOL is_file_exist(LPCTSTR csFile)
{
	DWORD dwAttrib = GetFileAttributes(csFile);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 == (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

// if the directory exists
BOOL is_dir_exist(LPCTSTR csDir)
{
	DWORD dwAttrib = GetFileAttributes(csDir);
	return INVALID_FILE_ATTRIBUTES != dwAttrib && 0 != (dwAttrib & FILE_ATTRIBUTE_DIRECTORY);
}

// if the file or directory exists
BOOL is_path_exist(LPCTSTR csPath)
{
	DWORD dwAttrib = GetFileAttributes(csPath);
	return INVALID_FILE_ATTRIBUTES != dwAttrib;
}

// create the directory recursively
void mkdir_recursively(const std::string& strPath)
{
	if (_access(strPath.c_str(), 0) != -1 || strPath.empty())
		return;

	mkdir_recursively(strPath.substr(0, strPath.rfind('\\')));
	mkdir(strPath.c_str());
}


std::vector<std::string> string_split(const std::string& str, const std::string& seperator)
{
	std::vector<std::string> result;
	if (seperator.empty())
	{
		result.push_back(str);
		return result;
	}

	std::size_t pos_begin = 0;
	std::size_t pos_seperator = str.find(seperator);
	while (pos_seperator != std::string::npos)
	{
		result.push_back(str.substr(pos_begin, pos_seperator - pos_begin));
		pos_begin = pos_seperator + seperator.length();
		pos_seperator = str.find(seperator, pos_begin);
	}

	result.push_back(str.substr(pos_begin));

	return result;
}

bool string_starts_with(const std::string& str, const std::string& substr)
{
	return str.find(substr) == 0;
}

bool string_ends_with(const std::string& str, const std::string& substr)
{
	size_t pos = str.rfind(substr);
	return (pos != std::string::npos) && (pos == str.length() - substr.length());
}

void string_trim_end(std::string& str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), str.end());
}

void string_trim_start(std::string& str)
{
	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

void string_trim(std::string& str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](int ch) {
		return !std::isspace(ch);
	}).base(), str.end());

	str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](int ch) {
		return !std::isspace(ch);
	}));
}

void string_to_lower(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void string_to_upper(std::string& str)
{
	transform(str.begin(), str.end(), str.begin(), ::toupper);
}

void string_replace(std::string& str, const std::string& search, const std::string& replacement)
{
	if (search == replacement)
	{
		return;
	}

	if (search == "")
	{
		return;
	}

	std::string::size_type pos;
	std::string::size_type lastPos = 0;
	while ((pos = str.find(search, lastPos)) != std::string::npos)
	{
		str.replace(pos, search.length(), replacement);
		lastPos = pos + replacement.length();
	}
}