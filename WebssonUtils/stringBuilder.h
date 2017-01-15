#pragma once

#include <string>
#include <sstream>

namespace webss
{
#define This StringBuilder
	class This
	{
	public:
		using size_type = std::string::size_type;

		This() {}

		This& operator+=(char c) { ss.put(c); }
		This& operator+=(std::string&& s) { ss << std::move(s); }
		This& operator+=(const std::string& s) { ss << s; }
		This& operator+=(const char* s) { ss << s; }
		This& operator+=(const This& o) { ss << o.ss.rdbuf(); }

		operator std::string() const { return ss.str(); }
		std::string str() { return ss.str(); }

	private:
		std::stringstream ss;
	};
#undef This

#define This SmartString
	class This
	{
	public:
		using size_type = std::string::size_type;

		This() {}

		This& operator+=(char c) { isValid = false; ss.put(c); }
		This& operator+=(std::string&& s) { isValid = false; ss << std::move(s); }
		This& operator+=(const std::string& s) { isValid = false; ss << s; }
		This& operator+=(const char* s)
		{
			isValid = false;
			for (char c; (c = *s) != '\0'; ++s)
				ss.put(c);
		}
		This& operator+=(const This& o) { isValid = false; ss << o.ss.rdbuf(); }

		size_type length() { return isValid ? s.length() : makeValid().length(); }

		operator std::string&() { return isValid ? s : makeValid(); }
		std::string& str() { return isValid ? s : makeValid(); }

	private:
		std::stringstream ss;
		std::string s;
		bool isValid;

		std::string& makeValid()
		{
			s = ss.str();
			return s;
		}
	};
#undef This
}