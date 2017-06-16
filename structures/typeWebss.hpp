//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

namespace webss
{
	class WebssType
	{
	public:
		enum class Enum
		{
			NONE, ENTITY, DEFAULT,
			PRIMITIVE_NULL, PRIMITIVE_BOOL, PRIMITIVE_INT, PRIMITIVE_DOUBLE, PRIMITIVE_STRING,
			DICTIONARY, LIST, LIST_TEXT, TUPLE, TUPLE_TEXT,
			THEAD, THEAD_BIN, THEAD_SELF, THEAD_STD, THEAD_TEXT,
			TEMPLATE_BIN, TEMPLATE_STD, TEMPLATE_TEXT,
			THEAD_PLUS_BIN, THEAD_PLUS_STD, THEAD_PLUS_TEXT,
			TEMPLATE_PLUS_BIN, TEMPLATE_PLUS_STD, TEMPLATE_PLUS_TEXT,
			DOCUMENT, NAMESPACE, ENUM
		};
		static const Enum NONE = Enum::NONE, ENTITY = Enum::ENTITY, DEFAULT = Enum::DEFAULT,
			PRIMITIVE_NULL = Enum::PRIMITIVE_NULL, PRIMITIVE_BOOL = Enum::PRIMITIVE_BOOL, PRIMITIVE_INT = Enum::PRIMITIVE_INT, PRIMITIVE_DOUBLE = Enum::PRIMITIVE_DOUBLE, PRIMITIVE_STRING = Enum::PRIMITIVE_STRING,
			DICTIONARY = Enum::DICTIONARY, LIST = Enum::LIST, LIST_TEXT = Enum::LIST_TEXT, TUPLE = Enum::TUPLE, TUPLE_TEXT = Enum::TUPLE_TEXT,
			THEAD = Enum::THEAD, THEAD_BIN = Enum::THEAD_BIN, THEAD_SELF = Enum::THEAD_SELF, THEAD_STD = Enum::THEAD_STD, THEAD_TEXT = Enum::THEAD_TEXT,
			TEMPLATE_BIN = Enum::TEMPLATE_BIN, TEMPLATE_STD = Enum::TEMPLATE_STD, TEMPLATE_TEXT = Enum::TEMPLATE_TEXT,
			THEAD_PLUS_BIN = Enum::THEAD_PLUS_BIN, THEAD_PLUS_STD = Enum::THEAD_PLUS_STD, THEAD_PLUS_TEXT = Enum::THEAD_PLUS_TEXT,
			TEMPLATE_PLUS_BIN = Enum::TEMPLATE_PLUS_BIN, TEMPLATE_PLUS_STD = Enum::TEMPLATE_PLUS_STD, TEMPLATE_PLUS_TEXT = Enum::TEMPLATE_PLUS_TEXT,
			DOCUMENT = Enum::DOCUMENT, NAMESPACE = Enum::NAMESPACE, ENUM = Enum::ENUM;

		constexpr WebssType() : e(Enum::NONE) {}
		constexpr WebssType(Enum e) : e(e) {}

		constexpr bool operator==(Enum o) const { return e == o; }
		constexpr bool operator==(WebssType o) const { return e == o.e; }
		constexpr bool operator!=(Enum o) const { return e != o; }
		constexpr bool operator!=(WebssType o) const { return e != o.e; }
		constexpr operator Enum() const { return e; }

		WebssType& operator=(Enum o);
		WebssType& operator=(WebssType o);

		std::string toString() const;
	private:
		const Enum e;
	};
}