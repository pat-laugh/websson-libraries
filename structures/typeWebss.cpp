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
	case WebssType::THEAD:
		return "template head";
	case WebssType::TEMPLATE:
		return "template";
	case WebssType::DOCUMENT:
		return "document";
	case WebssType::NAMESPACE:
		return "namespace";
	case WebssType::ENUM:
		return "enum";
	default:
		return "unknown";
	}
}