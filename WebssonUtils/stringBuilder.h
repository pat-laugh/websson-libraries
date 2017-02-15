//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <sstream>
#include <string>

namespace webss
{
#define This StringBuilder
	class This
	{
	public:
		This& operator+=(char c) { ss.put(c); return *this; }
		This& operator+=(std::string&& s) { ss << std::move(s); return *this; }
		This& operator+=(const std::string& s) { ss << s; return *this; }
		This& operator+=(const char* s) { ss << s; return *this; }
		This& operator+=(const This& o) { ss << o.ss.rdbuf(); return *this; }

		operator std::string() const { return ss.str(); }
		std::string str() { return ss.str(); }
	private:
		std::stringstream ss;
	};
#undef This
}