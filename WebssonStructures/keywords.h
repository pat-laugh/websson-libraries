//MIT License
//Copyright(c) 2016 Patrick Laughrea
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
			BOOL, INT1, INT2, INT4, INT8, DEC4, DEC8,
			STRING
		};
		static const Enum KEY_NULL = Enum::KEY_NULL, KEY_FALSE = Enum::KEY_FALSE, KEY_TRUE = Enum::KEY_TRUE,
			BOOL = Enum::BOOL, INT1 = Enum::INT1, INT2 = Enum::INT2, INT4 = Enum::INT4, INT8 = Enum::INT8, DEC4 = Enum::DEC4, DEC8 = Enum::DEC8,
			STRING = Enum::STRING;

		constexpr Keyword() : e(Enum::KEY_NULL) {}
		constexpr Keyword(Enum e) : e(e) {}

		constexpr bool operator==(Enum o) const { return e == o; }
		constexpr bool operator==(Keyword o) const { return e == o.e; }
		constexpr operator Enum() const { return e; }

		Keyword& operator=(Enum o) { const_cast<Enum&>(this->e) = o; return *this; }
		Keyword& operator=(Keyword o) { const_cast<Enum&>(this->e) = o.e; return *this; }

		//REQUIREMENT: s must be associated with a valid Keyword
		Keyword(std::string s);

		std::string toString() const;

		//if keyword is type, returns its size in bytes, else throws an error
		int getSize() const;

		//returns true if the keyword is a type: BOOL, INT1, INT2, INT4, INT8, DEC4, DEC8, STRING; else false
		bool isType() const;
	private:
		const Enum e;
	};

	//returns true if s is a keyword, else false
	bool isKeyword(const std::string& s);
}