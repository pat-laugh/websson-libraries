//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

namespace webss
{
	const char OPEN_DICTIONARY = '{';
	const char OPEN_LIST = '[';
	const char OPEN_TUPLE = '(';
	const char OPEN_TEMPLATE = '<';
	const char CLOSE_DICTIONARY = '}';
	const char CLOSE_LIST = ']';
	const char CLOSE_TUPLE = ')';
	const char CLOSE_TEMPLATE = '>';
	const char ASSIGN_CONTAINER_STRING[] = "::";

	class ConType
	{
	public:
		enum class Enum { DOCUMENT, DICTIONARY, LIST, TUPLE, THEAD };
		static const Enum DOCUMENT = Enum::DOCUMENT, DICTIONARY = Enum::DICTIONARY, LIST = Enum::LIST, TUPLE = Enum::TUPLE, THEAD = Enum::THEAD;

		constexpr ConType() : e(Enum::DOCUMENT) {}
		constexpr ConType(Enum e) : e(e) {}

		constexpr bool operator==(Enum o) const { return e == o; }
		constexpr bool operator==(ConType o) const { return e == o.e; }
		constexpr bool operator!=(Enum o) const { return e != o; }
		constexpr bool operator!=(ConType o) const { return e != o.e; }
		constexpr operator Enum() const { return e; }

		ConType& operator=(Enum o);
		ConType& operator=(ConType o);

		std::string toString() const;

		//returns true c == con's start char, else false
		bool isStart(char c) const;

		//returns true c == con's end char, else false
		bool isEnd(char c) const;

		//returns true if con has an end char, else false
		bool hasEndChar() const;
	private:
		const Enum e;
	};
}