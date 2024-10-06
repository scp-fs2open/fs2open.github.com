/* Copyright (c) 2012-2017 The ANTLR Project. All rights reserved.
 * Use of this file is governed by the BSD 3-clause license that
 * can be found in the LICENSE.txt file in the project root.
 */

#include "support/StringUtils.h"

namespace antlrcpp {

	// For all conversions utf8 <-> utf32.
	// VS 2015 and VS 2017 have different bugs in std::codecvt_utf8<char32_t> (VS 2013 works fine).
#if defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_VER < 2000
	typedef std::wstring_convert<std::codecvt_utf8<__int32>, __int32> UTF32Converter;
#else
	typedef std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> UTF32Converter;
#endif

	// The conversion functions fails in VS2017, so we explicitly use a workaround.
	template<typename T>
	std::string utf32_to_utf8(T const& data)
	{
		// Don't make the converter static or we have to serialize access to it.
		thread_local UTF32Converter converter;

#if defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_VER < 2000
		auto p = reinterpret_cast<const int32_t *>(data.data());
      return converter.to_bytes(p, p + data.size());
#else
		return converter.to_bytes(data);
#endif
	}

	template std::string utf32_to_utf8<UTF32String>(UTF32String const& data);

	UTF32String utf8_to_utf32(const char* first, const char* last)
	{
		thread_local UTF32Converter converter;

#if defined(_MSC_VER) && _MSC_VER >= 1900 && _MSC_VER < 2000
		auto r = converter.from_bytes(first, last);
      i32string s = reinterpret_cast<const int32_t *>(r.data());
#else
		std::u32string s = converter.from_bytes(first, last);
#endif

		return s;
	}

void replaceAll(std::string& str, std::string const& from, std::string const& to)
{
  if (from.empty())
    return;

  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'.
  }
}

std::string ws2s(std::wstring const& wstr) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::string narrow = converter.to_bytes(wstr);

  return narrow;
}

std::wstring s2ws(const std::string &str) {
  std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
  std::wstring wide = converter.from_bytes(str);
  
  return wide;
}

} // namespace antrlcpp
