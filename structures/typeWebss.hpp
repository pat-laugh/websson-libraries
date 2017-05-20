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
			TEMPLATE_HEAD_BINARY, TEMPLATE_HEAD_SCOPED, TEMPLATE_HEAD_SELF, TEMPLATE_HEAD_STANDARD, TEMPLATE_HEAD_TEXT,
			TEMPLATE_BINARY, TEMPLATE_SCOPED, TEMPLATE_STANDARD, TEMPLATE_TEXT,
			DOCUMENT, NAMESPACE, ENUM, BLOCK_HEAD, BLOCK, TUPLE_ABSTRACT
		};
		static const Enum NONE = Enum::NONE, ENTITY = Enum::ENTITY, DEFAULT = Enum::DEFAULT,
			PRIMITIVE_NULL = Enum::PRIMITIVE_NULL, PRIMITIVE_BOOL = Enum::PRIMITIVE_BOOL, PRIMITIVE_INT = Enum::PRIMITIVE_INT, PRIMITIVE_DOUBLE = Enum::PRIMITIVE_DOUBLE, PRIMITIVE_STRING = Enum::PRIMITIVE_STRING,
			DICTIONARY = Enum::DICTIONARY, LIST = Enum::LIST, LIST_TEXT = Enum::LIST_TEXT, TUPLE = Enum::TUPLE, TUPLE_TEXT = Enum::TUPLE_TEXT,
			TEMPLATE_HEAD_BINARY = Enum::TEMPLATE_HEAD_BINARY, TEMPLATE_HEAD_SCOPED = Enum::TEMPLATE_HEAD_SCOPED, TEMPLATE_HEAD_SELF = Enum::TEMPLATE_HEAD_SELF, TEMPLATE_HEAD_STANDARD = Enum::TEMPLATE_HEAD_STANDARD, TEMPLATE_HEAD_TEXT = Enum::TEMPLATE_HEAD_TEXT,
			TEMPLATE_BINARY = Enum::TEMPLATE_BINARY, TEMPLATE_SCOPED = Enum::TEMPLATE_SCOPED, TEMPLATE_STANDARD = Enum::TEMPLATE_STANDARD, TEMPLATE_TEXT = Enum::TEMPLATE_TEXT,
			DOCUMENT = Enum::DOCUMENT, NAMESPACE = Enum::NAMESPACE, ENUM = Enum::ENUM, BLOCK_HEAD = Enum::BLOCK_HEAD, BLOCK = Enum::BLOCK, TUPLE_ABSTRACT = Enum::TUPLE_ABSTRACT;

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