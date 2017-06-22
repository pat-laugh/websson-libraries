//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <sstream>
#include <string>

namespace webss
{
	class StringBuilder
	{
	public:
		StringBuilder() {}
		StringBuilder(StringBuilder&& o) : ss(std::move(o.ss)) {}
		StringBuilder(const StringBuilder& o) { put(o); }
		StringBuilder(char c) { put(c); }
		StringBuilder(std::string s) { put(std::move(s)); }
		StringBuilder(const char* s) { put(s); }

		StringBuilder& operator=(StringBuilder o) { ss = std::move(o.ss); return *this; }

		StringBuilder& put(const StringBuilder& o) { ss << o.str(); return *this; }
		StringBuilder& put(char c) { ss.put(c); return *this; }
		StringBuilder& put(std::string s) { ss << std::move(s); return *this; }
		StringBuilder& put(const char* s) { ss << s; return *this; }

		StringBuilder& operator+=(const StringBuilder& o) { return put(o); }
		StringBuilder& operator+=(char c) { return put(c); }
		StringBuilder& operator+=(std::string s) { return put(std::move(s)); }
		StringBuilder& operator+=(const char* s) { return put(s); }

		operator std::string() const { return ss.str(); }
		std::string str() const { return ss.str(); }
	private:
		std::ostringstream ss;
	};
}