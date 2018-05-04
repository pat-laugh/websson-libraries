//MIT License
//Copyright 2017-2018 Patrick Laughrea
#pragma once

namespace webss
{
	class WebssType
	{
	public:
		enum class Enum
		{
			NONE, ENTITY, DEFAULT,
			PRIMITIVE_NULL, PRIMITIVE_BOOL, PRIMITIVE_INT, PRIMITIVE_DOUBLE, PRIMITIVE_STRING, STRING_LIST,
			DICTIONARY, LIST, LIST_TEXT, TUPLE, TUPLE_TEXT,
			THEAD, TEMPLATE,
#ifdef COMPILE_WEBSS
			PRINT_STRING, PRINT_STRING_LIST,
#endif
			DOCUMENT, NAMESPACE, ENUM, PLACEHOLDER
		};
		static const Enum NONE = Enum::NONE, ENTITY = Enum::ENTITY, DEFAULT = Enum::DEFAULT,
			PRIMITIVE_NULL = Enum::PRIMITIVE_NULL, PRIMITIVE_BOOL = Enum::PRIMITIVE_BOOL, PRIMITIVE_INT = Enum::PRIMITIVE_INT, PRIMITIVE_DOUBLE = Enum::PRIMITIVE_DOUBLE, PRIMITIVE_STRING = Enum::PRIMITIVE_STRING, STRING_LIST = Enum::STRING_LIST,
			DICTIONARY = Enum::DICTIONARY, LIST = Enum::LIST, LIST_TEXT = Enum::LIST_TEXT, TUPLE = Enum::TUPLE, TUPLE_TEXT = Enum::TUPLE_TEXT,
			THEAD = Enum::THEAD, TEMPLATE = Enum::TEMPLATE,
#ifdef COMPILE_WEBSS
			PRINT_STRING = Enum::PRINT_STRING, PRINT_STRING_LIST = Enum::PRINT_STRING_LIST,
#endif
			DOCUMENT = Enum::DOCUMENT, NAMESPACE = Enum::NAMESPACE, ENUM = Enum::ENUM, PLACEHOLDER = Enum::PLACEHOLDER;

		constexpr WebssType() : e(Enum::NONE) {}
		constexpr WebssType(Enum e) : e(e) {}

		constexpr bool operator==(Enum o) const { return e == o; }
		constexpr bool operator==(WebssType o) const { return e == o.e; }
		constexpr bool operator!=(Enum o) const { return e != o; }
		constexpr bool operator!=(WebssType o) const { return e != o.e; }
		constexpr operator Enum() const { return e; }

		WebssType& operator=(Enum o);
		WebssType& operator=(WebssType o);

		const char* toString() const;
	private:
		const Enum e;
	};
}