//MIT License
//Copyright 2017-2018 Patrick Laughrea
#include "webss.hpp"

#include <cassert>

#include "dictionary.hpp"
#include "document.hpp"
#include "list.hpp"
#include "paramStandard.hpp"
#include "placeholder.hpp"
#include "template.hpp"
#include "tuple.hpp"

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
	default: assert(false);
	case Keyword::KEY_NULL:
		type = WebssType::PRIMITIVE_NULL;
		break;
	case Keyword::KEY_FALSE:
		tBool = false;
		break;
	case Keyword::KEY_TRUE:
		tBool = true;
		break;
	}
}
Webss::Webss(int tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(WebssInt tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(size_t tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(double tDouble) : type(WebssType::PRIMITIVE_DOUBLE), tDouble(tDouble) {}

Webss::Webss(const char* s) : type(WebssType::PRIMITIVE_STRING), tString(new string(s)) {}
Webss::Webss(string s) : type(WebssType::PRIMITIVE_STRING), tString(new string(move(s))) {}
#ifdef COMPILE_WEBSS
Webss::Webss(string s, WebssType type) : type(type), tString(new string(move(s))) {}
#endif
Webss::Webss(StringList s) : type(WebssType::STRING_LIST), stringList(new StringList(move(s))) {}

Webss::Webss(Document document) : type(WebssType::DOCUMENT), document(new Document(move(document))) {}
Webss::Webss(Dictionary dict) : type(WebssType::DICTIONARY), dict(new Dictionary(move(dict))) {}

Webss::Webss(Thead thead) : type(WebssType::THEAD), thead(new Thead(move(thead))) {}
Webss::Webss(Template templ) : type(WebssType::TEMPLATE), templ(new Template(move(templ))) {}

Webss::Webss(List list, WebssType type) : type(type), list(new List(move(list)))
{
	assert(type == WebssType::LIST || type == WebssType::LIST_TEXT);
}
Webss::Webss(Tuple tuple, WebssType type) : type(type), tuple(new Tuple(move(tuple)))
{
	assert(type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT);
}

Webss::Webss(Placeholder placeholder) : type(WebssType::PLACEHOLDER), placeholder(new Placeholder(move(placeholder))) {}

Webss::Webss(void* ptr, WebssType type) : type(type), ptr(ptr)
{
#ifndef NDEBUG
	switch (type)
	{
	default: assert(false);
	case WebssType::PRIMITIVE_STRING: case WebssType::STRING_LIST:
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING: case WebssType::PRINT_STRING_LIST:
#endif
	case WebssType::DOCUMENT: case WebssType::DICTIONARY:
	case WebssType::LIST: case WebssType::LIST_TEXT:
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
	case WebssType::THEAD: case WebssType::TEMPLATE: case WebssType::PLACEHOLDER:
	case WebssType::FOREACH: case WebssType::FOREACH_PARAM:
		break;
	}
#endif
}

void Webss::destroyUnion()
{
	switch (type)
	{
	default: assert(false);
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
	case WebssType::FOREACH_PARAM:
		break;
	case WebssType::ENTITY:
		ent.~Entity();
		break;
	case WebssType::DEFAULT:
		tDefault.~shared_ptr();
		break;
	case WebssType::PRIMITIVE_STRING:
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING:
#endif
		delete tString;
		break;
	case WebssType::STRING_LIST:
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING_LIST:
#endif
		delete stringList;
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
	case WebssType::PLACEHOLDER:
		delete placeholder;
		break;
	case WebssType::FOREACH:
		delete webss;
		break;
	}
	type = WebssType::NONE;
}

void Webss::copyUnion(Webss&& o)
{
	switch (o.type)
	{
	default: assert(false);
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL:
	case WebssType::FOREACH_PARAM:
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
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING:
#endif
		tString = o.tString;
		break;
	case WebssType::STRING_LIST:
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING_LIST:
#endif
		stringList = o.stringList;
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
	case WebssType::PLACEHOLDER:
		placeholder = o.placeholder;
		break;
	case WebssType::FOREACH:
		webss = o.webss;
		break;
	}
	type = o.type;
	o.type = WebssType::NONE;
}

void Webss::copyUnion(const Webss& o)
{
	switch (o.type)
	{
	default: assert(false);
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL:
	case WebssType::FOREACH_PARAM:
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
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING:
#endif
		tString = new string(*o.tString);
		break;
	case WebssType::STRING_LIST:
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING_LIST:
#endif
		stringList = new StringList(*o.stringList);
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
	case WebssType::PLACEHOLDER:
		placeholder = new Placeholder(*o.placeholder);
		break;
	case WebssType::FOREACH:
		webss = new Webss(*o.webss);
		break;
	}
	type = o.type;
}

bool Webss::operator==(const Webss& o) const
{
	if (this == &o)
		return true;
	if (getType() != o.getType())
		return false;
	const auto& w1 = getWebssLast(), w2 = o.getWebssLast();
	switch (w1.getTypeRaw())
	{
	default: assert(false);
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL:
	case WebssType::FOREACH_PARAM:
		return true;
	case WebssType::PRIMITIVE_BOOL:
		return w1.tBool == w2.tBool;
	case WebssType::PRIMITIVE_INT:
		return w1.tInt == w2.tInt;
	case WebssType::PRIMITIVE_DOUBLE:
		return w1.tDouble == w2.tDouble;
	case WebssType::PRIMITIVE_STRING:
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING:
#endif
		return *w1.tString == *w2.tString;
	case WebssType::STRING_LIST:
#ifdef COMPILE_WEBSS
	case WebssType::PRINT_STRING_LIST:
#endif
		return *w1.stringList == *w2.stringList;
	case WebssType::DOCUMENT:
		return *w1.document == *w2.document;
	case WebssType::DICTIONARY:
		return *w1.dict == *w2.dict;
	case WebssType::LIST: case WebssType::LIST_TEXT:
		return *w1.list == *w2.list;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		return *w1.tuple == *w2.tuple;
	case WebssType::THEAD:
		return *w1.thead == *w2.thead;
	case WebssType::TEMPLATE:
		return *w1.templ == *w2.templ;
	case WebssType::NAMESPACE:
		return w1.nspace == w2.nspace;
	case WebssType::ENUM:
		return w1.tEnum == w2.tEnum;
	case WebssType::FOREACH:
		return *w1.webss == *w2.webss;
	}
}
bool Webss::operator!=(const Webss& o) const { return !(*this == o); }

static const char* ERROR_ACCESS = "can't access ";
static const char* ERROR_ACCESS_INDEX = " with an index";
static const char* ERROR_ACCESS_KEY = " with a key";

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
	case WebssType::PLACEHOLDER:
		return placeholder->getValue()[index];
	default:
		throw runtime_error(string(ERROR_ACCESS) + type.toString() + ERROR_ACCESS_INDEX);
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
	case WebssType::PLACEHOLDER:
		return placeholder->getValue()[key];
	default:
		throw runtime_error(string(ERROR_ACCESS) + type.toString() + ERROR_ACCESS_KEY);
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
	case WebssType::PLACEHOLDER:
		return placeholder->getValue().at(index);
	default:
		throw runtime_error(string(ERROR_ACCESS) + type.toString() + ERROR_ACCESS_INDEX);
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
	case WebssType::PLACEHOLDER:
		return placeholder->getValue().at(key);
	default:
		throw runtime_error(string(ERROR_ACCESS) + type.toString() + ERROR_ACCESS_KEY);
	}
}

const Webss& Webss::getWebssLast() const
{
	if (type == WebssType::ENTITY)
		return ent.getContent().getWebssLast();
	else if (type == WebssType::DEFAULT)
		return tDefault->getWebssLast();
	else if (type == WebssType::PLACEHOLDER)
		return placeholder->getValue();
	else
		return *this;
}

WebssType Webss::getType() const
{
	return getWebssLast().getTypeRaw();
}

static string errorMessageGet(WebssType expected, WebssType actual)
{
	return string("could not get ") + expected.toString() + "; instead, type was " + actual.toString();
}

#define PatternGetConstSafe(Type, Func) do { \
const auto& webss = getWebssLast(); \
if (webss.getTypeRaw() == Type) \
	return webss.Func(); \
throw runtime_error(errorMessageGet(Type, webss.getTypeRaw())); } while (false)

bool Webss::getBool() const { PatternGetConstSafe(WebssType::PRIMITIVE_BOOL, getBoolRaw); }
WebssInt Webss::getInt() const { PatternGetConstSafe(WebssType::PRIMITIVE_INT, getIntRaw); }
double Webss::getDouble() const { PatternGetConstSafe(WebssType::PRIMITIVE_DOUBLE, getDoubleRaw); }
const Document& Webss::getDocument() const { PatternGetConstSafe(WebssType::DOCUMENT, getDocumentRaw); }
const Dictionary& Webss::getDictionary() const { PatternGetConstSafe(WebssType::DICTIONARY, getDictionaryRaw); }
const Thead& Webss::getThead() const { PatternGetConstSafe(WebssType::THEAD, getTheadRaw); }
const Template& Webss::getTemplate() const { PatternGetConstSafe(WebssType::TEMPLATE, getTemplateRaw); }
const Namespace& Webss::getNamespace() const { PatternGetConstSafe(WebssType::NAMESPACE, getNamespaceRaw); }
const Enum& Webss::getEnum() const { PatternGetConstSafe(WebssType::ENUM, getEnumRaw); }

#ifndef COMPILE_WEBSS
const std::string& Webss::getString() const { PatternGetConstSafe(WebssType::PRIMITIVE_STRING, getStringRaw); }
const StringList& Webss::getStringList() const { PatternGetConstSafe(WebssType::STRING_LIST, getStringListRaw); }
#else
const std::string& Webss::getString() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::PRIMITIVE_STRING || type == WebssType::PRINT_STRING)
		return webss.getStringRaw();
	throw runtime_error(errorMessageGet(WebssType::PRIMITIVE_STRING, type));
}
const StringList& Webss::getStringList() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::STRING_LIST || type == WebssType::PRINT_STRING_LIST)
		return webss.getStringListRaw();
	throw runtime_error(errorMessageGet(WebssType::STRING_LIST, type));
}
#endif

const List& Webss::getList() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::LIST || type == WebssType::LIST_TEXT)
		return webss.getListRaw();
	throw runtime_error(errorMessageGet(WebssType::LIST, type));
}
const Tuple& Webss::getTuple() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT)
		return webss.getTupleRaw();
	throw runtime_error(errorMessageGet(WebssType::TUPLE, type));
}

bool Webss::isNone() const { return getType() == WebssType::NONE; }
bool Webss::isNull() const { return getType() == WebssType::PRIMITIVE_NULL; }
bool Webss::isBool() const { return getType() == WebssType::PRIMITIVE_BOOL; }
bool Webss::isInt() const { return getType() == WebssType::PRIMITIVE_INT; }
bool Webss::isDouble() const { return getType() == WebssType::PRIMITIVE_DOUBLE; }
bool Webss::isString() const { return getType() == WebssType::PRIMITIVE_STRING; }
bool Webss::isStringList() const { return getType() == WebssType::STRING_LIST; }
#ifdef COMPILE_WEBSS
bool Webss::isPrintString() const { return getType() == WebssType::PRINT_STRING; }
bool Webss::isPrintStringList() const { return getType() == WebssType::PRINT_STRING_LIST; }
#endif
bool Webss::isDocument() const { return getType() == WebssType::DOCUMENT; }
bool Webss::isDictionary() const { return getType() == WebssType::DICTIONARY; }
bool Webss::isNamespace() const { return getType() == WebssType::NAMESPACE; }
bool Webss::isEnum() const { return getType() == WebssType::ENUM; }

bool Webss::isListText() const { return getType() == WebssType::LIST_TEXT; }
bool Webss::isTupleText() const { return getType() == WebssType::TUPLE_TEXT; }
bool Webss::isThead() const { return getType() == WebssType::THEAD; }
bool Webss::isTemplate() const { return getType() == WebssType::TEMPLATE; }

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

bool Webss::isAbstract() const
{
	switch (type)
	{
	case WebssType::NONE: assert(false);
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
const Placeholder& Webss::getPlaceholderRaw() const { assert(getTypeRaw() == WebssType::PLACEHOLDER); return *placeholder; }

bool Webss::getBoolRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_BOOL); return tBool; }
WebssInt Webss::getIntRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_INT); return tInt; }
double Webss::getDoubleRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_DOUBLE); return tDouble; }
#ifndef COMPILE_WEBSS
const std::string& Webss::getStringRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_STRING); return *tString; }
const StringList& Webss::getStringListRaw() const { assert(getTypeRaw() == WebssType::STRING_LIST); return *stringList; }
#else
const std::string& Webss::getStringRaw() const { assert(getTypeRaw() == WebssType::PRIMITIVE_STRING || getTypeRaw() == WebssType::PRINT_STRING); return *tString; }
const StringList& Webss::getStringListRaw() const { assert(getTypeRaw() == WebssType::STRING_LIST || getTypeRaw() == WebssType::PRINT_STRING_LIST); return *stringList; }
#endif
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
Placeholder& Webss::getPlaceholderRaw() { assert(getTypeRaw() == WebssType::PLACEHOLDER); return *placeholder; }

#ifndef COMPILE_WEBSS
std::string& Webss::getStringRaw() { assert(getTypeRaw() == WebssType::PRIMITIVE_STRING); return *tString; }
StringList& Webss::getStringListRaw() { assert(getTypeRaw() == WebssType::STRING_LIST); return *stringList; }
#else
std::string& Webss::getStringRaw() { assert(getTypeRaw() == WebssType::PRIMITIVE_STRING || getTypeRaw() == WebssType::PRINT_STRING); return *tString; }
StringList& Webss::getStringListRaw() { assert(getTypeRaw() == WebssType::STRING_LIST || getTypeRaw() == WebssType::PRINT_STRING_LIST); return *stringList; }
#endif

Document& Webss::getDocumentRaw() { assert(getTypeRaw() == WebssType::DOCUMENT); return *document; }
Dictionary& Webss::getDictionaryRaw() { assert(getTypeRaw() == WebssType::DICTIONARY); return *dict; }
List& Webss::getListRaw() { assert(getTypeRaw() == WebssType::LIST || getTypeRaw() == WebssType::LIST_TEXT); return *list; }
Tuple& Webss::getTupleRaw() { assert(getTypeRaw() == WebssType::TUPLE || getTypeRaw() == WebssType::TUPLE_TEXT); return *tuple; }
Thead& Webss::getTheadRaw() { assert(getTypeRaw() == WebssType::THEAD); return *thead; }
Template& Webss::getTemplateRaw() { assert(getTypeRaw() == WebssType::TEMPLATE); return *templ; }