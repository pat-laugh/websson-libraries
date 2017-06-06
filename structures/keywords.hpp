//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

namespace webss
{
	class Keyword
	{
	public:
		enum class Enum
		{
			KEY_NULL, KEY_FALSE, KEY_TRUE,
			BOOL, INT8, INT16, INT32, INT64, FLOAT, DOUBLE,
			STRING
		};
		static const Enum KEY_NULL = Enum::KEY_NULL, KEY_FALSE = Enum::KEY_FALSE, KEY_TRUE = Keyword::Enum::KEY_TRUE,
			BOOL = Enum::BOOL, INT8 = Enum::INT8, INT16 = Enum::INT16, INT32 = Enum::INT32, INT64 = Enum::INT64, FLOAT = Enum::FLOAT, DOUBLE = Enum::DOUBLE,
			STRING = Enum::STRING;

		constexpr Keyword() : e(Enum::KEY_NULL) {}
		constexpr Keyword(Enum e) : e(e) {}

		constexpr bool operator==(Enum o) const { return e == o; }
		constexpr bool operator==(Keyword o) const { return e == o.e; }
		constexpr operator Enum() const { return e; }

		Keyword& operator=(Enum o);
		Keyword& operator=(Keyword o);

		//REQUIREMENT: s must be associated with a valid Keyword
		Keyword(std::string s);

		std::string toString() const;

	private:
		const Enum e;
	};

	//returns true if s is a keyword, else false
	bool isKeyword(const std::string& s);
}