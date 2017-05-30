//MIT License
//Copyright 2017 Patrick Laughrea
#include "typeWebss.hpp"

using namespace std;
using namespace webss;

WebssType& WebssType::operator=(Enum o) { const_cast<Enum&>(this->e) = o; return *this; }
WebssType& WebssType::operator=(WebssType o) { const_cast<Enum&>(this->e) = o.e; return *this; }

string WebssType::toString() const
{
	switch (e)
	{
	case WebssType::NONE:
		return "none";
	case WebssType::ENTITY:
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
	case WebssType::LIST_TEXT:
		return "list text";
	case WebssType::TUPLE:
		return "tuple";
	case WebssType::TUPLE_TEXT:
		return "tuple text";
	case WebssType::TEMPLATE_HEAD_BINARY:
		return "template head binary";
	case WebssType::TEMPLATE_HEAD_SELF:
		return "template head self";
	case WebssType::TEMPLATE_HEAD_STANDARD:
		return "template head standard";
	case WebssType::TEMPLATE_HEAD_TEXT:
		return "template head text";
	case WebssType::TEMPLATE_BINARY:
		return "template binary";
	case WebssType::TEMPLATE_STANDARD:
		return "template standard";
	case WebssType::TEMPLATE_TEXT:
		return "template text";
	case WebssType::DOCUMENT:
		return "document";
	case WebssType::NAMESPACE:
		return "namespace";
	case WebssType::ENUM:
		return "enum";
	case WebssType::BLOCK_HEAD:
		return "block head";
	case WebssType::BLOCK:
		return "block";
	case WebssType::DICTIONARY_ABSTRACT:
		return "dictionary abstract";
	case WebssType::LIST_ABSTRACT:
		return "list abstract";
	case WebssType::TUPLE_ABSTRACT:
		return "tuple abstract";
	case WebssType::LIST_TEXT_ABSTRACT:
		return "list text abstract";
	case WebssType::TUPLE_TEXT_ABSTRACT:
		return "tuple text abstract";
	default:
		return "unknown";
	}
}