// Stuff that can't be included in vmallocator.h

#include "globalincs/vmallocator.h"

std::locale SCP_default_locale("");

void SCP_tolower(SCP_string &str)
{
	std::for_each(str.begin(), str.end(), [](char &ch) { ch = SCP_tolower(ch); });
}

void SCP_toupper(SCP_string &str)
{
	std::for_each(str.begin(), str.end(), [](char &ch) { ch = SCP_toupper(ch); });
}

void SCP_tolower(char *str)
{
	for (; *str != '\0'; ++str)
		*str = SCP_tolower(*str);
}

void SCP_toupper(char *str)
{
	for (; *str != '\0'; ++str)
		*str = SCP_toupper(*str);
}

// in-place modification of string to title case; this is a bit naive but it is good enough for the time being
void SCP_totitle(char *str)
{
	bool prev_alpha = false;

	for (; *str != '\0'; ++str)
	{
		bool this_alpha = (*str >= 'a' && *str <= 'z') || (*str >= 'A' && *str <= 'Z');

		if (this_alpha)
		{
			if (prev_alpha)
				*str = SCP_tolower(*str);
			else
				*str = SCP_toupper(*str);
		}

		prev_alpha = this_alpha;
	}
}

// in-place modification of string to title case; same naive algorithm as above
void SCP_totitle(SCP_string &str)
{
	SCP_string title_str;
	bool prev_alpha = false;

	std::for_each(str.begin(), str.end(), [&prev_alpha](char &ch)
	{
		bool this_alpha = (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');

		if (this_alpha)
		{
			if (prev_alpha)
				ch = SCP_tolower(ch);
			else
				ch = SCP_toupper(ch);
		}

		prev_alpha = this_alpha;
	});
}

bool SCP_truncate(SCP_string &str, size_t len)
{
	if (str.length() > len)
	{
		str.resize(len);
		return true;
	}
	else
		return false;
}

bool lcase_equal(const SCP_string& _Left, const SCP_string& _Right)
{
	if (_Left.size() != _Right.size())
		return false;

	auto l_it = _Left.cbegin();
	auto r_it = _Right.cbegin();

	while (l_it != _Left.cend())
	{
		if (SCP_tolower(*l_it) != SCP_tolower(*r_it))
			return false;

		++l_it;
		++r_it;
	}

	return true;
}

bool lcase_lessthan(const SCP_string& _Left, const SCP_string& _Right)
{
	auto l_it = _Left.cbegin();
	auto r_it = _Right.cbegin();

	while (true)
	{
		if (l_it == _Left.cend())
			return (r_it != _Right.cend());
		else if (r_it == _Right.cend())
			return false;

		auto lch = SCP_tolower(*l_it);
		auto rch = SCP_tolower(*r_it);

		if (lch < rch)
			return true;
		else if (lch > rch)
			return false;

		++l_it;
		++r_it;
	}

	return true;
}
