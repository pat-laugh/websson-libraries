//MIT License
//Copyright 2017 Patrick Laughrea
#include "webss.hpp"

#include <cassert>

#include "dictionary.hpp"
#include "list.hpp"
#include "tuple.hpp"
#include "template.hpp"
#include "document.hpp"
#include "paramStandard.hpp"

using namespace std;
using namespace webss;

Webss::Webss() {}
Webss::~Webss() { destroyUnion(); }

Webss::Webss(Webss&& o) { copyUnion(move(o)); }
Webss::Webss(const Webss& o) { copyUnion(o); }

Webss& Webss::operator=(Webss o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}

Webss::Webss(const Entity& ent) : type(WebssType::ENTITY), ent(ent) {}
Webss::Webss(const Default& tDefault) : type(WebssType::DEFAULT), tDefault(tDefault) { assert(tDefault != nullptr); }
Webss::Webss(const Namespace& nspace) : type(WebssType::NAMESPACE), nspace(nspace) {}
Webss::Webss(const Enum& tEnum) : type(WebssType::ENUM), tEnum(tEnum) {}

Webss::Webss(bool tBool) : type(WebssType::PRIMITIVE_BOOL), tBool(tBool) {}
Webss::Webss(Keyword keyword) : type(WebssType::PRIMITIVE_BOOL)
{
	switch (keyword)
	{
	case Keyword::KEY_NULL:
		type = WebssType::PRIMITIVE_NULL;
		break;
	case Keyword::KEY_FALSE:
		tBool = false;
		break;
	case Keyword::KEY_TRUE:
		tBool = true;
		break;
	default:
		assert(false);
	}
}
Webss::Webss(int tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(WebssInt tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(size_t tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(double tDouble) : type(WebssType::PRIMITIVE_DOUBLE), tDouble(tDouble) {}

Webss::Webss(const char* s) : type(WebssType::PRIMITIVE_STRING), tString(new string(s)) {}
Webss::Webss(string s) : type(WebssType::PRIMITIVE_STRING), tString(new string(move(s))) {}

Webss::Webss(Document document) : type(WebssType::DOCUMENT), document(new Document(move(document))) {}
Webss::Webss(Dictionary dict) : type(WebssType::DICTIONARY), dict(new Dictionary(move(dict))) {}

Webss::Webss(Thead thead) : type(WebssType::THEAD), thead(new Thead(move(thead))) {}

Webss::Webss(List list, WebssType type) : type(type), list(new List(move(list)))
{
	assert(type == WebssType::LIST || type == WebssType::LIST_TEXT);
}
Webss::Webss(Tuple tuple, WebssType type) : type(type), tuple(new Tuple(move(tuple)))
{
	assert(type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT);
}
Webss::Webss(Template templ) : type(WebssType::TEMPLATE), templ(new Template(move(templ))) {}

void Webss::destroyUnion()
{
	switch (type)
	{
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		break;
	case WebssType::ENTITY:
		ent.~Entity();
		break;
	case WebssType::DEFAULT:
		tDefault.~shared_ptr();
		break;
	case WebssType::PRIMITIVE_STRING:
		delete tString;
		break;
	case WebssType::DOCUMENT:
		delete document;
		break;
	case WebssType::DICTIONARY:
		delete dict;
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT:
		delete list;
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		delete tuple;
		break;
	case WebssType::THEAD:
		delete thead;
		break;
	case WebssType::TEMPLATE:
		delete templ;
		break;
	case WebssType::NAMESPACE:
		nspace.~Namespace();
		break;
	case WebssType::ENUM:
		tEnum.~Enum();
		break;
	default:
		assert(false);
	}
	type = WebssType::NONE;
}

void Webss::copyUnion(Webss&& o)
{
	switch (o.type)
	{
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL:
		break;
	case WebssType::ENTITY:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case WebssType::DEFAULT:
		new (&tDefault) Default(move(o.tDefault));
		o.tDefault.~shared_ptr();
		break;
	case WebssType::PRIMITIVE_BOOL:
		tBool = o.tBool;
		break;
	case WebssType::PRIMITIVE_INT:
		tInt = o.tInt;
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		tDouble = o.tDouble;
		break;

	case WebssType::PRIMITIVE_STRING:
		tString = o.tString;
		break;
	case WebssType::DOCUMENT:
		document = o.document;
		break;
	case WebssType::DICTIONARY:
		dict = o.dict;
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT:
		list = o.list;
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		tuple = o.tuple;
		break;
	case WebssType::THEAD:
		thead = o.thead;
		break;
	case WebssType::TEMPLATE:
		templ = o.templ;
		break;
	case WebssType::NAMESPACE:
		new (&nspace) Namespace(move(o.nspace));
		o.nspace.~Namespace();
		break;
	case WebssType::ENUM:
		new (&tEnum) Enum(move(o.tEnum));
		o.tEnum.~Enum();
		break;
	default:
		assert(false);
	}
	type = o.type;
	o.type = WebssType::NONE;
}

void Webss::copyUnion(const Webss& o)
{
	switch (o.type)
	{
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL:
		break;
	case WebssType::ENTITY:
		new (&ent) Entity(o.ent);
		break;
	case WebssType::DEFAULT:
		new (&tDefault) Default(o.tDefault);
		break;
	case WebssType::PRIMITIVE_BOOL:
		tBool = o.tBool;
		break;
	case WebssType::PRIMITIVE_INT:
		tInt = o.tInt;
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		tDouble = o.tDouble;
		break;

	case WebssType::PRIMITIVE_STRING:
		tString = new string(*o.tString);
		break;
	case WebssType::DOCUMENT:
		document = new Document(*o.document);
		break;
	case WebssType::DICTIONARY:
		dict = new Dictionary(*o.dict);
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT:
		list = new List(*o.list);
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		tuple = new Tuple(*o.tuple);
		break;
	case WebssType::THEAD:
		thead = new Thead(*o.thead);
		break;
	case WebssType::TEMPLATE:
		templ = new Template(*o.templ);
		break;
	case WebssType::NAMESPACE:
		new (&nspace) Namespace(o.nspace);
		break;
	case WebssType::ENUM:
		new (&tEnum) Enum(o.tEnum);
		break;
	default:
		assert(false);
	}
	type = o.type;
}


bool Webss::operator==(const Webss& o) const
{
	if (this == &o)
		return true;
	if (type != o.type)
		return false;
	switch (o.type)
	{
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL:
		return true;
	case WebssType::ENTITY:
		return ent == o.ent;
	case WebssType::DEFAULT:
		return *tDefault == *o.tDefault;
	case WebssType::PRIMITIVE_BOOL:
		return tBool == o.tBool;
	case WebssType::PRIMITIVE_INT:
		return tInt == o.tInt;
	case WebssType::PRIMITIVE_DOUBLE:
		return tDouble == o.tDouble;
	case WebssType::PRIMITIVE_STRING:
		return *tString == *o.tString;
	case WebssType::DOCUMENT:
		return *document == *o.document;
	case WebssType::DICTIONARY:
		return *dict == *o.dict;
	case WebssType::LIST: case WebssType::LIST_TEXT:
		return *list == *o.list;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return *tuple == *o.tuple;
	case WebssType::THEAD:
		return *thead == *o.thead;
	case WebssType::TEMPLATE:
		return *templ == *o.templ;
	case WebssType::NAMESPACE:
		return nspace == o.nspace;
	case WebssType::ENUM:
		return tEnum == o.tEnum;
	default:
		assert(false); throw domain_error("");
	}
}
bool Webss::operator!=(const Webss& o) const { return !(*this == o); }

const char ERROR_ACCESS[] = "can't access ";
const char ERROR_ACCESS_INDEX[] = " with an index";
const char ERROR_ACCESS_KEY[] = " with a key";

const Webss& Webss::operator[](int index) const
{
	assert(isConcrete());
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent()[index];
	case WebssType::DEFAULT:
		return (*tDefault)[index];
	case WebssType::LIST: case WebssType::LIST_TEXT:
		return (*list)[index];
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return (*tuple)[index];
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_INDEX);
	}
}

const Webss& Webss::operator[](const std::string& key) const
{
	assert(isConcrete());
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent()[key];
	case WebssType::DEFAULT:
		return (*tDefault)[key];
	case WebssType::DICTIONARY:
		return (*dict)[key];
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return (*tuple)[key];
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_KEY);
	}
}

const Webss& Webss::at(int index) const
{
	assert(isConcrete());
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().at(index);
	case WebssType::DEFAULT:
		return tDefault->at(index);
	case WebssType::LIST: case WebssType::LIST_TEXT:
		return list->at(index);
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return tuple->at(index);
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_INDEX);
	}
}

const Webss& Webss::at(const std::string& key) const
{
	assert(isConcrete());
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().at(key);
	case WebssType::DEFAULT:
		return tDefault->at(key);
	case WebssType::DICTIONARY:
		return dict->at(key);
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return tuple->at(key);
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_KEY);
	}
}

const Webss& Webss::getWebssLast() const
{
	if (type == WebssType::ENTITY)
		return ent.getContent().getWebssLast();
	else if (type == WebssType::DEFAULT)
		return tDefault->getWebssLast();
	else
		return *this;
}

WebssType Webss::getType() const
{
	return getWebssLast().getTypeRaw();
}

string errorMessageGet(WebssType expected, WebssType actual)
{
	return "could not get " + expected.toString() + "; instead webss type was " + actual.toString();
}

#define PATTERN_GET_CONST_SAFE(Type, Func) \
const auto& webss = getWebssLast(); \
if (webss.getTypeRaw() == Type) \
	return webss.Func(); \
else \
	throw runtime_error(errorMessageGet(Type, webss.getTypeRaw()));

bool Webss::getBool() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_BOOL, getBoolRaw); }
WebssInt Webss::getInt() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_INT, getIntRaw); }
double Webss::getDouble() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_DOUBLE, getDoubleRaw); }
const std::string& Webss::getString() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_STRING, getStringRaw); }
const Document& Webss::getDocument() const { PATTERN_GET_CONST_SAFE(WebssType::DOCUMENT, getDocumentRaw); }
const Dictionary& Webss::getDictionary() const { PATTERN_GET_CONST_SAFE(WebssType::DICTIONARY, getDictionaryRaw); }
const Thead& Webss::getThead() const { PATTERN_GET_CONST_SAFE(WebssType::THEAD, getTheadRaw); }
const Template& Webss::getTemplate() const { PATTERN_GET_CONST_SAFE(WebssType::TEMPLATE, getTemplateRaw); }
const Namespace& Webss::getNamespace() const { PATTERN_GET_CONST_SAFE(WebssType::NAMESPACE, getNamespaceRaw); }
const Enum& Webss::getEnum() const { PATTERN_GET_CONST_SAFE(WebssType::ENUM, getEnumRaw); }

const List& Webss::getList() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::LIST || type == WebssType::LIST_TEXT)
		return webss.getListRaw();
	else
		throw runtime_error(errorMessageGet(WebssType::LIST, type));
}
const Tuple& Webss::getTuple() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT)
		return webss.getTupleRaw();
	else
		throw runtime_error(errorMessageGet(WebssType::TUPLE, type));
}

bool Webss::isNone() const { return getType() == WebssType::NONE; }
bool Webss::isNull() const { return getType() == WebssType::PRIMITIVE_NULL; }
bool Webss::isBool() const { return getType() == WebssType::PRIMITIVE_BOOL; }
bool Webss::isInt() const { return getType() == WebssType::PRIMITIVE_INT; }
bool Webss::isDouble() const { return getType() == WebssType::PRIMITIVE_DOUBLE; }
bool Webss::isString() const { return getType() == WebssType::PRIMITIVE_STRING; }
bool Webss::isDocument() const { return getType() == WebssType::DOCUMENT; }
bool Webss::isDictionary() const { return getType() == WebssType::DICTIONARY; }
bool Webss::isNamespace() const { return getType() == WebssType::NAMESPACE; }
bool Webss::isEnum() const { return getType() == WebssType::ENUM; }

bool Webss::isListText() const { return getType() == WebssType::LIST_TEXT; }
bool Webss::isTupleText() const { return getType() == WebssType::TUPLE_TEXT; }
bool Webss::isThead() const { return getType() == WebssType::THEAD; }

bool Webss::isList() const
{
	const auto type = getType();
	return type == WebssType::LIST || type == WebssType::LIST_TEXT;
}
bool Webss::isTuple() const
{
	const auto type = getType();
	return type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT;
}

bool Webss::isTemplate() const
{
	const auto type = getType();
	return type == WebssType::TEMPLATE;
}

bool Webss::isAbstract() const
{
	switch (type)
	{
	case WebssType::NONE:
		assert(false);
	case WebssType::ENTITY:
		return ent.getContent().isAbstract();
	case WebssType::DEFAULT:
		return tDefault->isAbstract();
	case WebssType::THEAD: case WebssType::NAMESPACE: case WebssType::ENUM:
		return true;
	default:
		return false;
	}
}

bool Webss::isConcrete() const
{
	return !isAbstract();
}

WebssType Webss::getTypeRaw() const { return type; }

const Entity& Webss::getEntityRaw() const { assert(getTypeRaw() == WebssType::ENTITY); return ent; }
const Default& Webss::getDefaultRaw() const { assert(getTypeRaw() == WebssType::DEFAULT); return tDefault; }
const Namespace& Webss::getNamespaceRaw() const { assert(getTypeRaw() == WebssType::NAMESPACE); return nspace; }
const Enum& Webss::getEnumRaw() const { assert(getTypeRaw() == WebssType::ENUM); return tEnum; }

bool Webss::getBoolRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_BOOL); return tBool; }
WebssInt Webss::getIntRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_INT); return tInt; }
double Webss::getDoubleRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_DOUBLE); return tDouble; }
const std::string& Webss::getStringRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_STRING); return *tString; }
const Document& Webss::getDocumentRaw() const { assert(getTypeRaw() == WebssType::DOCUMENT); return *document; }
const Dictionary& Webss::getDictionaryRaw() const { assert(getTypeRaw() == WebssType::DICTIONARY); return *dict; }
const List& Webss::getListRaw() const { assert(getTypeRaw() == WebssType::LIST || getTypeRaw() == WebssType::LIST_TEXT); return *list; }
const Tuple& Webss::getTupleRaw() const { assert(getTypeRaw() == WebssType::TUPLE || getTypeRaw() == WebssType::TUPLE_TEXT); return *tuple; }
const Thead& Webss::getTheadRaw() const { assert(getTypeRaw() == WebssType::THEAD); return *thead; }
const Template& Webss::getTemplateRaw() const { assert(getTypeRaw() == WebssType::TEMPLATE); return *templ; }

Entity& Webss::getEntityRaw() { assert(getTypeRaw() == WebssType::ENTITY); return ent; }
Default& Webss::getDefaultRaw() { assert(getTypeRaw() == WebssType::DEFAULT); return tDefault; }
Namespace& Webss::getNamespaceRaw() { assert(getTypeRaw() == WebssType::NAMESPACE); return nspace; }
Enum& Webss::getEnumRaw() { assert(getTypeRaw() == WebssType::ENUM); return tEnum; }

std::string& Webss::getStringRaw() { assert(getTypeRaw() == WebssType::PRIMITIVE_STRING); return *tString; }
Document& Webss::getDocumentRaw() { assert(getTypeRaw() == WebssType::DOCUMENT); return *document; }
Dictionary& Webss::getDictionaryRaw() { assert(getTypeRaw() == WebssType::DICTIONARY); return *dict; }
List& Webss::getListRaw() { assert(getTypeRaw() == WebssType::LIST || getTypeRaw() == WebssType::LIST_TEXT); return *list; }
Tuple& Webss::getTupleRaw() { assert(getTypeRaw() == WebssType::TUPLE || getTypeRaw() == WebssType::TUPLE_TEXT); return *tuple; }
Thead& Webss::getTheadRaw() { assert(getTypeRaw() == WebssType::THEAD); return *thead; }
Template& Webss::getTemplateRaw() { assert(getTypeRaw() == WebssType::TEMPLATE); return *templ; }