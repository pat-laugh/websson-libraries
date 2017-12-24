//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

namespace webss
{
	class ConType
	{
	public:
		enum class Enum {
			DOCUMENT, DICTIONARY, LIST, TUPLE, THEAD,
			
			TEMPLATE_BIN = DICTIONARY, TEMPLATE_BIN_ARRAY = LIST,
			TEMPLATE_FUN = TUPLE,
		};
		static const Enum DOCUMENT = Enum::DOCUMENT, DICTIONARY = Enum::DICTIONARY,
				LIST = Enum::LIST, TUPLE = Enum::TUPLE, THEAD = Enum::THEAD,
				TEMPLATE_BIN = Enum::TEMPLATE_BIN, TEMPLATE_BIN_ARRAY = Enum::TEMPLATE_BIN_ARRAY,
				TEMPLATE_FUN = Enum::TEMPLATE_FUN;

		constexpr ConType() : e(Enum::DOCUMENT) {}
		constexpr ConType(Enum e) : e(e) {}

		constexpr bool operator==(Enum o) const { return e == o; }
		constexpr bool operator==(ConType o) const { return e == o.e; }
		constexpr bool operator!=(Enum o) const { return e != o; }
		constexpr bool operator!=(ConType o) const { return e != o.e; }
		constexpr operator Enum() const { return e; }

		ConType& operator=(Enum o);
		ConType& operator=(ConType o);

		const char* toString() const;

		//returns true c == con's start char, else false
		bool isStart(char c) const;

		//returns true c == con's end char, else false
		bool isEnd(char c) const;

		//returns true if con has an end char, else false
		bool hasEndChar() const;
		
		//returns the null char if con does not have a start char
		char getStartChar() const;
		
		//returns the null char if con does not have an end char
		char getEndChar() const;
		
	private:
		const Enum e;
	};
}