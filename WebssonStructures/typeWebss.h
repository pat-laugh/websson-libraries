//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

namespace webss
{
	class WebssType
	{
	public:
		enum class Enum
		{
			NONE, VARIABLE, DEFAULT,
			PRIMITIVE_NULL, PRIMITIVE_BOOL, PRIMITIVE_INT, PRIMITIVE_DOUBLE, PRIMITIVE_STRING,
			DICTIONARY, LIST, TUPLE,
			FUNCTION_HEAD, FUNCTION_HEAD_STANDARD, FUNCTION_HEAD_BINARY, FUNCTION_HEAD_SCOPED,
			FUNCTION_STANDARD, FUNCTION_BINARY, FUNCTION_SCOPED,
			DOCUMENT, BLOCK_VALUE, BLOCK_ID, NAMESPACE, ENUM, BLOCK_HEAD
		};
		static const Enum NONE = Enum::NONE, VARIABLE = Enum::VARIABLE, DEFAULT = Enum::DEFAULT,
			PRIMITIVE_NULL = Enum::PRIMITIVE_NULL, PRIMITIVE_BOOL = Enum::PRIMITIVE_BOOL, PRIMITIVE_INT = Enum::PRIMITIVE_INT, PRIMITIVE_DOUBLE = Enum::PRIMITIVE_DOUBLE, PRIMITIVE_STRING = Enum::PRIMITIVE_STRING,
			DICTIONARY = Enum::DICTIONARY, LIST = Enum::LIST, TUPLE = Enum::TUPLE,
			FUNCTION_HEAD = Enum::FUNCTION_HEAD, FUNCTION_HEAD_STANDARD = Enum::FUNCTION_HEAD_STANDARD, FUNCTION_HEAD_BINARY = Enum::FUNCTION_HEAD_BINARY, FUNCTION_HEAD_SCOPED = Enum::FUNCTION_HEAD_SCOPED,
			FUNCTION_STANDARD = Enum::FUNCTION_STANDARD, FUNCTION_BINARY = Enum::FUNCTION_BINARY, FUNCTION_SCOPED = Enum::FUNCTION_SCOPED,
			DOCUMENT = Enum::DOCUMENT, BLOCK_VALUE = Enum::BLOCK_VALUE, BLOCK_ID = Enum::BLOCK_ID, NAMESPACE = Enum::NAMESPACE, ENUM = Enum::ENUM, BLOCK_HEAD = Enum::BLOCK_HEAD;

		constexpr WebssType() : e(Enum::NONE) {}
		constexpr WebssType(Enum e) : e(e) {}

		constexpr bool operator==(Enum o) const { return e == o; }
		constexpr bool operator==(WebssType o) const { return e == o.e; }
		constexpr bool operator!=(Enum o) const { return e != o; }
		constexpr bool operator!=(WebssType o) const { return e != o.e; }
		constexpr operator Enum() const { return e; }

		WebssType& operator=(Enum o) { const_cast<Enum&>(this->e) = o; return *this; }
		WebssType& operator=(WebssType o) { const_cast<Enum&>(this->e) = o.e; return *this; }

		std::string toString() const
		{
			switch (e)
			{
			case WebssType::NONE:
				return "none";
			case WebssType::VARIABLE:
				return "entity";
			case WebssType::DEFAULT:
				return "default";
			case WebssType::PRIMITIVE_NULL:
				return "null";
			case WebssType::PRIMITIVE_BOOL:
				return "bool";
			case WebssType::PRIMITIVE_INT:
				return "int";
			case WebssType::PRIMITIVE_DOUBLE:
				return "double";
			case WebssType::PRIMITIVE_STRING:
				return "string";
			case WebssType::DICTIONARY:
				return "dictionary";
			case WebssType::LIST:
				return "list";
			case WebssType::TUPLE:
				return "tuple";
			case WebssType::FUNCTION_HEAD:
				return "function head";
			case WebssType::FUNCTION_HEAD_STANDARD:
				return "function head standard";
			case WebssType::FUNCTION_HEAD_BINARY:
				return "function head binary";
			case WebssType::FUNCTION_HEAD_SCOPED:
				return "function head scoped";
			case WebssType::FUNCTION_STANDARD:
				return "function standard";
			case WebssType::FUNCTION_BINARY:
				return "function binary";
			case WebssType::FUNCTION_SCOPED:
				return "function scoped";
			case WebssType::DOCUMENT:
				return "document";
			case WebssType::BLOCK_ID:
				return "block id";
			case WebssType::BLOCK_VALUE:
				return "block value";
			case WebssType::NAMESPACE:
				return "namespace";
			case WebssType::ENUM:
				return "enum";
			case WebssType::BLOCK_HEAD:
				return "block head";
			default:
				return "unknown";
			}
		}
	private:
		const Enum e;
	};
}