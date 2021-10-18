#include "common_utf8.h"

#include <locale>
#include <codecvt>
#include <windows.h>

namespace
{
	class chs_codecvt : public std::codecvt_byname<wchar_t, char, std::mbstate_t>
	{
	public:
		// you can create a gbk codecvt by name 'chs' or '.936' on windows platform
		// or 'zh_CN.GBK' on linux platform, and 'zh_cn.gb2312' on macOS platform
		chs_codecvt() : codecvt_byname("chs")
		{ }
	};

	static inline bool has_utf8_bom(const char *in_char)
	{
		uint8_t *in = (uint8_t *)in_char;
		return (in && in[0] == 0xef && in[1] == 0xbb && in[2] == 0xbf);
	}
}

bool is_utf8_chars(const uint8_t *str)
{
	const uint8_t *byte;
	uint32_t codepoint;
	uint32_t min;

	while (*str)
	{
		byte = str;
		codepoint = *(byte++);
		uint32_t top = (codepoint & 0x80) >> 1;
		if ((codepoint & 0xc0) == 0x80 || codepoint >= 0xFE)
		{
			return false;
		}
		while (codepoint & top)
		{
			int tmp = *(byte++) - 128;
			if (tmp >> 6)
			{
				return false;
			}
			codepoint = (codepoint << 6) + tmp;
			top <<= 5;
		}
		codepoint &= (top << 1) - 1;

		min = byte - str == 1 ? 0 : byte - str == 2 ? 0x80 : 1 << (5 * (byte - str) - 4);
		if (codepoint < min || codepoint >= 0x110000 ||
			codepoint == 0xFFFE ||
			codepoint >= 0xD800 && codepoint <= 0xDFFF)
		{
			return false;
		}

		str = byte;
	}

	return true;
}

bool is_utf8_str(const std::string& str)
{
	//the character bytes count
	char bytesCount = 0;
	unsigned char chr;
	bool allAreAscii = true;

	std::string::const_iterator it;
	for (it = str.begin(); it != str.end(); it++)
	{
		chr = *it;

		if ((chr & 0x80) != 0)
		{
			allAreAscii = false;
		}

		if (bytesCount == 0)
		{
			if (chr >= 0x80)
			{
				if (chr >= 0xFC && chr <= 0xFD)
				{
					bytesCount = 6;
				}
				else if (chr >= 0xF8)
				{
					bytesCount = 5;
				}
				else if (chr >= 0xF0)
				{
					bytesCount = 4;
				}
				else if (chr >= 0xE0)
				{
					bytesCount = 3;
				}
				else if (chr >= 0xC0)
				{
					bytesCount = 2;
				}
				else
				{
					return false;
				}
				bytesCount--;
			}
		}
		else
		{
			if ((chr & 0xC0) != 0x80)
			{
				return false;
			}
			bytesCount--;
		}
	}

	//left bytes breaks the utf8 rules
	if (bytesCount > 0)
	{
		return false;
	}

	//all characters are ascii-coded, they are utf8
	if (allAreAscii)
	{
		return true;
	}

	return true;
}

std::string gbk_to_utf8(const std::string& str)
{
	std::wstring_convert<chs_codecvt> converterGBK;
	std::wstring tmp_wstr = converterGBK.from_bytes(str);

	std::wstring_convert<std::codecvt_utf8<wchar_t>> converterUTF8;
	return converterUTF8.to_bytes(tmp_wstr);
}

std::string utf8_to_gbk(const std::string& str)
{
	std::wstring_convert<std::codecvt_utf8<wchar_t>> converterUTF8;
	std::wstring tmp_wstr = converterUTF8.from_bytes(str);

	std::wstring_convert<chs_codecvt> converterGBK;
	return converterGBK.to_bytes(tmp_wstr);
}

size_t utf8_to_wchar(const char *in, size_t in_size, wchar_t *out,
	size_t out_size)
{
	int insize = (int)in_size;
	int ret;

	if (insize == 0)
	{
		insize = (int)strlen(in);
	}

	/* prevent bom from being used in the string */
	if (has_utf8_bom(in)) 
	{
		if (insize >= 3)
		{
			in += 3;
			insize -= 3;
		}
	}

	ret = MultiByteToWideChar(CP_UTF8, 0, in, insize, out, (int)out_size);

	return (ret > 0) ? (size_t)ret : 0;
}

size_t wchar_to_utf8(const wchar_t *in, size_t in_size, char *out,
	size_t out_size)
{
	int insize = (int)in_size;
	int ret;

	if (insize == 0)
	{
		insize = (int)wcslen(in);
	}

	ret = WideCharToMultiByte(CP_UTF8, 0, in, insize, out, (int)out_size,
		NULL, NULL);

	return (ret > 0) ? (size_t)ret : 0;
}
