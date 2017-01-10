//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "webss.h"

using namespace std;
using namespace webss;

const char ERROR_UNDEFINED[] = "undefined behavior";
const char ERROR_ADD_KEY[] = "can't add key-value to ";
const char ERROR_ADD_VALUE[] = "can't add value-only to ";

Webss::Webss() : t(WebssType::NONE) {}
Webss::Webss(WebssType t) : t(t)
{
	switch (t)
	{
	case WebssType::NONE: case WebssType::DEFAULT: case WebssType::PRIMITIVE_NULL:
		break;
	case WebssType::DICTIONARY:
		dictionary = new Dictionary();
		break;
	case WebssType::LIST:
		list = new List();
		break;
	case WebssType::TUPLE:
		tuple = new Tuple();
		break;
	default:
		throw domain_error("can't make empty Webss object of type: " + t.toString());
	}
}
Webss::Webss(WebssType t, bool containerText) : t(t)
{
	switch (t)
	{
	case WebssType::LIST:
		list = new List(containerText);
		break;
	case WebssType::TUPLE:
		tuple = new Tuple(containerText);
		break;
	default:
		throw domain_error("can't make text container of type: " + t.toString());
	}
}

Webss::Webss(WebssType::Enum t) : Webss(WebssType(t)) {}
Webss::Webss(WebssType::Enum t, bool containerText) : Webss(WebssType(t), containerText) {}

Webss::Webss(bool tBool) : t(WebssType::PRIMITIVE_BOOL), tBool(tBool) {}
Webss::Webss(Keyword keyword)
{
	switch (keyword)
	{
	case Keyword::KEY_NULL:
		t = WebssType::PRIMITIVE_NULL;
		break;
	case Keyword::KEY_FALSE:
		t = WebssType::PRIMITIVE_BOOL;
		tBool = false;
		break;
	case Keyword::KEY_TRUE:
		t = WebssType::PRIMITIVE_BOOL;
		tBool = true;
		break;
	case Keyword::BOOL: case Keyword::INT1: case Keyword::INT2: case Keyword::INT4: case Keyword::INT8: case Keyword::DEC4: case Keyword::DEC8:
		t = WebssType::PRIMITIVE_INT;
		tInt = keyword.getSize();
		break;
	default:
		throw domain_error("unknow keyword: " + keyword.toString());
	}
}
Webss::Webss(int tInt) : t(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(type_int tInt) : t(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(size_t tInt) : t(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(double tDouble) : t(WebssType::PRIMITIVE_DOUBLE), tDouble(tDouble) {}

Webss::Webss(const char* s) : t(WebssType::PRIMITIVE_STRING), tString(new string(s)) {}

#define PATTERN_CONSTRUCT_MOVE(type, name, con) \
Webss::Webss(type&& name) : t(WebssType::con), name(new type(move(name))) {}

PATTERN_CONSTRUCT_MOVE(string, tString, PRIMITIVE_STRING)
PATTERN_CONSTRUCT_MOVE(Document, document, DOCUMENT)
PATTERN_CONSTRUCT_MOVE(Dictionary, dictionary, DICTIONARY)
PATTERN_CONSTRUCT_MOVE(List, list, LIST)
PATTERN_CONSTRUCT_MOVE(Tuple, tuple, TUPLE)
PATTERN_CONSTRUCT_MOVE(FunctionHeadStandard, fheadStandard, FUNCTION_HEAD_STANDARD)
PATTERN_CONSTRUCT_MOVE(FunctionHeadBinary, fheadBinary, FUNCTION_HEAD_BINARY)
PATTERN_CONSTRUCT_MOVE(FunctionStandard, funcStandard, FUNCTION_STANDARD)
PATTERN_CONSTRUCT_MOVE(FunctionBinary, funcBinary, FUNCTION_BINARY)
//PATTERN_CONSTRUCT_MOVE(FunctionScoped, funcScoped, FUNCTION_SCOPED)
PATTERN_CONSTRUCT_MOVE(BlockId, blockId, BLOCK_ID)
PATTERN_CONSTRUCT_MOVE(Block, block, BLOCK_VALUE)
PATTERN_CONSTRUCT_MOVE(Namespace, nspace, NAMESPACE)
Webss::Webss(Enum&& name, bool) : t(WebssType::ENUM), nspace(new Namespace(move(name))) {}

#define PATTERN_CONSTRUCT_CONST(type, name, con) \
Webss::Webss(const type& name) : t(WebssType::con), name(new type(name)) {}

PATTERN_CONSTRUCT_CONST(string, tString, PRIMITIVE_STRING)
PATTERN_CONSTRUCT_CONST(Document, document, DOCUMENT)
PATTERN_CONSTRUCT_CONST(Dictionary, dictionary, DICTIONARY)
PATTERN_CONSTRUCT_CONST(List, list, LIST)
PATTERN_CONSTRUCT_CONST(Tuple, tuple, TUPLE)
PATTERN_CONSTRUCT_CONST(FunctionHeadStandard, fheadStandard, FUNCTION_HEAD_STANDARD)
PATTERN_CONSTRUCT_CONST(FunctionHeadBinary, fheadBinary, FUNCTION_HEAD_BINARY)
PATTERN_CONSTRUCT_CONST(FunctionStandard, funcStandard, FUNCTION_STANDARD)
PATTERN_CONSTRUCT_CONST(FunctionBinary, funcBinary, FUNCTION_BINARY)
//PATTERN_CONSTRUCT_CONST(FunctionScoped, funcScoped, FUNCTION_SCOPED)
PATTERN_CONSTRUCT_CONST(BlockId, blockId, BLOCK_ID)
PATTERN_CONSTRUCT_CONST(Block, block, BLOCK_VALUE)
PATTERN_CONSTRUCT_CONST(Namespace, nspace, NAMESPACE)
Webss::Webss(const Enum& name, bool) : t(WebssType::ENUM), nspace(new Namespace(name)) {}

Webss::Webss(FunctionHeadStandard&& head, Webss&& body) : t(WebssType::FUNCTION_STANDARD)
{
	switch (body.t)
	{
	case WebssType::LIST:
		funcStandard = new FunctionStandard(move(head), move(*body.list));
		break;
	case WebssType::TUPLE:
		funcStandard = new FunctionStandard(move(head), move(*body.tuple));
		break;
	default:
		throw domain_error(ERROR_UNDEFINED);
	}
}

Webss::Webss(Webss&& o) { copyUnion(move(o)); }
Webss::Webss(const Webss& o) { copyUnion(o); }
Webss::~Webss() { destroyUnion(); }

void Webss::destroyUnion()
{
	switch (t)
	{
	case WebssType::PRIMITIVE_STRING:
		delete tString;
		break;
	case WebssType::DOCUMENT:
		delete document;
		break;
	case WebssType::DICTIONARY:
		delete dictionary;
		break;
	case WebssType::LIST:
		delete list;
		break;
	case WebssType::TUPLE:
		delete tuple;
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		delete fheadStandard;
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		delete fheadBinary;
		break;
	case WebssType::FUNCTION_STANDARD:
		delete funcStandard;
		break;
	case WebssType::FUNCTION_BINARY:
		delete funcBinary;
		break;
//	case WebssType::FUNCTION_SCOPED:
//		delete funcScoped;
//		break;
	case WebssType::BLOCK_ID:
		delete blockId;
		break;
	case WebssType::BLOCK_VALUE:
		delete block;
		break;
	case WebssType::NAMESPACE: case WebssType::ENUM:
		delete nspace;
		break;
	case WebssType::VARIABLE:
		var.~BasicVariable();
		break;
	case WebssType::DEFAULT:
		tDefault.~shared_ptr();
		break;
	default:
		break;
	}
}

Webss& Webss::operator=(Webss&& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(move(o));
	}
	return *this;
}

Webss& Webss::operator=(const Webss& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(o);
	}
	return *this;
}

void Webss::copyUnion(Webss&& o)
{
	switch (t = o.t)
	{
	default:
		return;
	case WebssType::PRIMITIVE_BOOL:
		tBool = o.tBool;
		return;
	case WebssType::PRIMITIVE_INT:
		tInt = o.tInt;
		return;
	case WebssType::PRIMITIVE_DOUBLE:
		tDouble = o.tDouble;
		return;

	case WebssType::PRIMITIVE_STRING:
		tString = o.tString;
		break;
	case WebssType::DOCUMENT:
		document= o.document;
		break;
	case WebssType::DICTIONARY:
		dictionary = o.dictionary;
		break;
	case WebssType::LIST:
		list = o.list;
		break;
	case WebssType::TUPLE:
		tuple = o.tuple;
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		fheadStandard = o.fheadStandard;
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		fheadBinary = o.fheadBinary;
		break;
	case WebssType::FUNCTION_STANDARD:
		funcStandard = o.funcStandard;
		break;
	case WebssType::FUNCTION_BINARY:
		funcBinary = o.funcBinary;
		break;
//	case WebssType::FUNCTION_SCOPED:
//		funcScoped = o.funcScoped;
//		break;
	case WebssType::BLOCK_ID:
		blockId = o.blockId;
		break;
	case WebssType::BLOCK_VALUE:
		block = o.block;
		break;
	case WebssType::NAMESPACE: case WebssType::ENUM:
		nspace = o.nspace;
		break;
	case WebssType::VARIABLE:
		new (&var) Variable(move(o.var));
		break;
	case WebssType::DEFAULT:
		new (&tDefault) Default(move(o.tDefault));
		break;
	}
	o.t = WebssType::NONE;
}

void Webss::copyUnion(const Webss& o)
{
	switch (t = o.t)
	{
	default:
		return;
	case WebssType::PRIMITIVE_BOOL:
		tBool = o.tBool;
		return;
	case WebssType::PRIMITIVE_INT:
		tInt = o.tInt;
		return;
	case WebssType::PRIMITIVE_DOUBLE:
		tDouble = o.tDouble;
		return;

	case WebssType::PRIMITIVE_STRING:
		tString = new string(*o.tString);
		break;
	case WebssType::DOCUMENT:
		document = new Document(*o.document);
		break;
	case WebssType::DICTIONARY:
		dictionary = new Dictionary(*o.dictionary);
		break;
	case WebssType::LIST:
		list = new List(*o.list);
		break;
	case WebssType::TUPLE:
		tuple = new Tuple(*o.tuple);
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		fheadStandard = new FunctionHeadStandard(*o.fheadStandard);
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		fheadBinary = new FunctionHeadBinary(*o.fheadBinary);
		break;
	case WebssType::FUNCTION_STANDARD:
		funcStandard = new FunctionStandard(*o.funcStandard);
		break;
	case WebssType::FUNCTION_BINARY:
		funcBinary = new FunctionBinary(*o.funcBinary);
		break;
//	case WebssType::FUNCTION_SCOPED:
//		funcScoped = new FunctionScoped(*o.funcScoped);
//		break;
	case WebssType::BLOCK_ID:
		blockId = new BlockId(*o.blockId);
		break;
	case WebssType::BLOCK_VALUE:
		block = new Block(*o.block);
		break;
	case WebssType::NAMESPACE: case WebssType::ENUM:
		nspace = new Namespace(*o.nspace);
		break;
	case WebssType::VARIABLE:
		new (&var) Variable(o.var);
		break;
	case WebssType::DEFAULT:
		new (&tDefault) Default(o.tDefault);
		break;
	}
}

void Webss::add(Webss&& value)
{
	switch (t)
	{
	case WebssType::TUPLE:
		tuple->add(move(value));
		break;
	case WebssType::LIST:
		list->add(move(value));
		break;
	default:
		throw domain_error(ERROR_ADD_VALUE + t.toString());
	}
}
void Webss::add(const Webss& value)
{
	switch (t)
	{
	case WebssType::TUPLE:
		tuple->add(value);
		break;
	case WebssType::LIST:
		list->add(value);
		break;
	default:
		throw domain_error(ERROR_ADD_VALUE + t.toString());
	}
}

void Webss::add(const string& key, const Webss& value)
{
	switch (t)
	{
	case WebssType::TUPLE:
		tuple->add(key, value);
		break;
	case WebssType::DICTIONARY:
		dictionary->add(key, value);
		break;
	default:
		throw domain_error(ERROR_ADD_KEY + t.toString());
	}
}
void Webss::addSafe(const string& key, const Webss& value)
{
	switch (t)
	{
	case WebssType::TUPLE:
		tuple->addSafe(key, value);
		break;
	case WebssType::DICTIONARY:
		dictionary->addSafe(key, value);
		break;
	default:
		throw domain_error(ERROR_ADD_KEY + t.toString());
	}
}

void Webss::add(string&& key, Webss&& value)
{
	switch (t)
	{
	case WebssType::TUPLE:
		tuple->add(move(key), move(value));
		break;
	case WebssType::DICTIONARY:
		dictionary->add(move(key), move(value));
		break;
	default:
		throw domain_error(ERROR_ADD_KEY + t.toString());
	}
}
void Webss::addSafe(string&& key, Webss&& value)
{
	switch (t)
	{
	case WebssType::TUPLE:
		tuple->addSafe(move(key), move(value));
		break;
	case WebssType::DICTIONARY:
		dictionary->addSafe(move(key), move(value));
		break;
	default:
		throw domain_error(ERROR_ADD_KEY + t.toString());
	}
}

const char ERROR_ACCESS[] = "can't access ";
const char ERROR_ACCESS_INDEX[] = " with an index";
const char ERROR_ACCESS_KEY[] = " with a key";

const Webss& Webss::operator[](int index) const
{
	switch (t)
	{
	case WebssType::LIST:
		return (*list)[index];
	case WebssType::TUPLE:
		return (*tuple)[index];
	case WebssType::FUNCTION_STANDARD:
		return (*funcStandard)[index];
	case WebssType::FUNCTION_BINARY:
		return (*funcBinary)[index];
//	case WebssType::FUNCTION_SCOPED:
//		return (*funcScoped)[index];
	case WebssType::VARIABLE:
		return var.getContent()[index];
	case WebssType::DEFAULT:
		return (*tDefault)[index];
	default:
		throw runtime_error(ERROR_ACCESS + t.toString() + ERROR_ACCESS_INDEX);
	}
}

const Webss& Webss::operator[](const std::string& key) const
{
	switch (t)
	{
	case WebssType::DICTIONARY:
		return (*dictionary)[key];
	case WebssType::TUPLE:
		return (*tuple)[key];
	case WebssType::FUNCTION_STANDARD:
		return (*funcStandard)[key];
	case WebssType::FUNCTION_BINARY:
		return (*funcBinary)[key];
//	case WebssType::FUNCTION_SCOPED:
//		return (*funcScoped)[key];
	case WebssType::VARIABLE:
		return var.getContent()[key];
	case WebssType::DEFAULT:
		return (*tDefault)[key];
	default:
		throw runtime_error(ERROR_ACCESS + t.toString() + ERROR_ACCESS_KEY);
	}
}

const Webss& Webss::at(int index) const
{
	switch (t)
	{
	case WebssType::LIST:
		return list->at(index);
	case WebssType::TUPLE:
		return tuple->at(index);
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->at(index);
	case WebssType::FUNCTION_BINARY:
		return funcBinary->at(index);
//	case WebssType::FUNCTION_SCOPED:
//		return funcScoped->at(index);
	case WebssType::VARIABLE:
		return var.getContent().at(index);
	case WebssType::DEFAULT:
		return tDefault->at(index);
	default:
		throw runtime_error(ERROR_ACCESS + t.toString() + ERROR_ACCESS_INDEX);
	}
}

const Webss& Webss::at(const std::string& key) const
{
	switch (t)
	{
	case WebssType::DICTIONARY:
		return dictionary->at(key);
	case WebssType::TUPLE:
		return tuple->at(key);
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->at(key);
	case WebssType::FUNCTION_BINARY:
		return funcBinary->at(key);
//	case WebssType::FUNCTION_SCOPED:
//		return funcScoped->at(key);
	case WebssType::VARIABLE:
		return var.getContent().at(key);
	case WebssType::DEFAULT:
		return tDefault->at(key);
	default:
		throw runtime_error(ERROR_ACCESS + t.toString() + ERROR_ACCESS_KEY);
	}
}

bool Webss::hasVariable() const { return t == WebssType::VARIABLE; }

const char ERROR_COULD_NOT_GETs1[] = "tried to get ";
const char ERROR_COULD_NOT_GETs2[] = " but webss type was ";
#define PATTERN_GET_CONST(x, y, z) \
switch (t) \
{ \
	case WebssType::VARIABLE: \
		return var.getContent().y; \
	case WebssType::DEFAULT: \
		return tDefault->y; \
	case z: \
		return x; \
	default: \
		throw runtime_error(ERROR_COULD_NOT_GETs1 + WebssType(z).toString() + ERROR_COULD_NOT_GETs2 + t.toString()); \
}

WebssType Webss::getType() const
{
	switch (t)
	{
	case WebssType::VARIABLE:
		return var.getContent().getType();
	case WebssType::DEFAULT:
		return tDefault->getType();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->hasList ? WebssType::LIST : WebssType::TUPLE;
	case WebssType::FUNCTION_BINARY:
		return WebssType::TUPLE;
//	case WebssType::FUNCTION_SCOPED:
//		return WebssType::TUPLE;
	default:
		return t;
	}
}

bool Webss::getBool() const { PATTERN_GET_CONST(tBool, getBool(), WebssType::PRIMITIVE_BOOL); }
double Webss::getDouble() const { PATTERN_GET_CONST(tDouble, getDouble(), WebssType::PRIMITIVE_DOUBLE); }
const std::string& Webss::getString() const { PATTERN_GET_CONST(*tString, getString(), WebssType::PRIMITIVE_STRING); }
const Dictionary& Webss::getDictionary() const { PATTERN_GET_CONST(*dictionary, getDictionary(), WebssType::DICTIONARY); }
const Document& Webss::getDocument() const { PATTERN_GET_CONST(*document, getDocument(), WebssType::DOCUMENT); }
const FunctionHeadStandard& Webss::getFunctionHeadStandard() const { PATTERN_GET_CONST(*fheadStandard, getFunctionHeadStandard(), WebssType::FUNCTION_HEAD_STANDARD); }
const FunctionHeadBinary& Webss::getFunctionHeadBinary() const { PATTERN_GET_CONST(*fheadBinary, getFunctionHeadBinary(), WebssType::FUNCTION_HEAD_BINARY); }
const FunctionStandard& Webss::getFunctionStandard() const { PATTERN_GET_CONST(*funcStandard, getFunctionStandard(), WebssType::FUNCTION_STANDARD); }
const FunctionBinary& Webss::getFunctionBinary() const { PATTERN_GET_CONST(*funcBinary, getFunctionBinary(), WebssType::FUNCTION_BINARY); }
//const FunctionScoped& Webss::getFunctionScoped() const { PATTERN_GET_CONST(*funcScoped, getFunctionScoped(), WebssType::FUNCTION_SCOPED); }
const BlockId& Webss::getBlockId() const { PATTERN_GET_CONST(*blockId, getBlockId(), WebssType::BLOCK_ID); }
const Block& Webss::getBlock() const { PATTERN_GET_CONST(*block, getBlock(), WebssType::BLOCK_VALUE); }
const Namespace& Webss::getNamespace() const { PATTERN_GET_CONST(*nspace, getNamespace(), WebssType::NAMESPACE); }
const Enum& Webss::getEnum() const { PATTERN_GET_CONST(*nspace, getEnum(), WebssType::ENUM); }

type_int Webss::getInt() const
{
	switch (t)
	{
	case WebssType::VARIABLE:
		return var.getContent().getInt();
	case WebssType::DEFAULT:
		return tDefault->getInt();
	case WebssType::PRIMITIVE_INT:
		return tInt;
	default:
		throw runtime_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::PRIMITIVE_INT).toString() + ERROR_COULD_NOT_GETs2 + t.toString());

	}
}
const List& Webss::getList() const
{
	switch (t)
	{
	case WebssType::VARIABLE:
		return var.getContent().getList();
	case WebssType::DEFAULT:
		return tDefault->getList();
	case WebssType::LIST:
		return *list;
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->getList();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::LIST).toString() + ERROR_COULD_NOT_GETs2 + t.toString());

	}
}
const Tuple& Webss::getTuple() const
{
	switch (t)
	{
	case WebssType::VARIABLE:
		return var.getContent().getTuple();
	case WebssType::DEFAULT:
		return tDefault->getTuple();
	case WebssType::TUPLE:
		return *tuple;
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->getTuple();
	case WebssType::FUNCTION_BINARY:
		return funcBinary->getTuple();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::TUPLE).toString() + ERROR_COULD_NOT_GETs2 + t.toString());
	}
}

#define PATTERN_IS(x, y) \
switch (t) \
{ \
	case WebssType::VARIABLE: \
		return var.getContent().y; \
	case WebssType::DEFAULT: \
		return tDefault->y; \
	case x: \
		return true; \
	default: \
		return false; \
}

bool Webss::isNone() const { PATTERN_IS(WebssType::NONE, isNone()) }
bool Webss::isNull() const { PATTERN_IS(WebssType::PRIMITIVE_NULL, isNull()) }
bool Webss::isBool() const { PATTERN_IS(WebssType::PRIMITIVE_BOOL, isBool()) }
bool Webss::isInt() const { PATTERN_IS(WebssType::PRIMITIVE_INT, isInt()) }
bool Webss::isDouble() const { PATTERN_IS(WebssType::PRIMITIVE_DOUBLE, isDouble()) }
bool Webss::isString() const { PATTERN_IS(WebssType::PRIMITIVE_STRING, isString()) }
bool Webss::isDictionary() const { PATTERN_IS(WebssType::DICTIONARY, isDictionary()) }
bool Webss::isDocument() const { PATTERN_IS(WebssType::DOCUMENT, isDocument()) }
bool Webss::isFunctionHeadStandard() const { PATTERN_IS(WebssType::FUNCTION_HEAD_STANDARD, isFunctionHeadStandard()) }
bool Webss::isFunctionHeadBinary() const { PATTERN_IS(WebssType::FUNCTION_HEAD_BINARY, isFunctionHeadBinary()) }
bool Webss::isBlockId() const { PATTERN_IS(WebssType::BLOCK_ID, isBlockId()) }
bool Webss::isBlock() const { PATTERN_IS(WebssType::BLOCK_VALUE, isBlock()) }
bool Webss::isNamespace() const { PATTERN_IS(WebssType::NAMESPACE, isNamespace()) }
bool Webss::isEnum() const { PATTERN_IS(WebssType::ENUM, isEnum()) }

bool Webss::isList() const
{
	switch (t)
	{
	case WebssType::VARIABLE:
		return var.getContent().isList();
	case WebssType::DEFAULT:
		return tDefault->isList();
	case WebssType::LIST:
		return true;
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->hasList;
	default:
		return false;
	}
}
bool Webss::isTuple() const
{
	switch (t)
	{
	case WebssType::VARIABLE:
		return var.getContent().isTuple();
	case WebssType::DEFAULT:
		return tDefault->isTuple();
	case WebssType::TUPLE:
		return true;
	case WebssType::FUNCTION_STANDARD:
		return !funcStandard->hasList;
	case WebssType::FUNCTION_BINARY:
		return true;
	default:
		return false;
	}
}


bool Webss::isFunctionHead() const
{
	switch (t)
	{
	case WebssType::FUNCTION_HEAD_BINARY: case WebssType::FUNCTION_HEAD_MANDATORY: case WebssType::FUNCTION_HEAD_STANDARD:
		return true;
	default:
		return false;
	}
}

bool Webss::isPrimitive() const
{
	switch (t)
	{
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE: case WebssType::PRIMITIVE_STRING:
		return true;
	case WebssType::VARIABLE:
		return var.getContent().isPrimitive();
	case WebssType::DEFAULT:
		return tDefault->isPrimitive();
	default:
		return false;
	}
}

bool Webss::isValue() const
{
	switch (t)
	{
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE: case WebssType::PRIMITIVE_STRING:
	case WebssType::DICTIONARY: case WebssType::LIST: case WebssType::TUPLE:
	case WebssType::FUNCTION_STANDARD: case WebssType::FUNCTION_BINARY: case WebssType::FUNCTION_SCOPED: case WebssType::FUNCTION_MANDATORY:
	case WebssType::BLOCK_VALUE:
		return true;
	case WebssType::VARIABLE:
		return var.getContent().isValue();
	case WebssType::DEFAULT:
		return tDefault->isValue();
	default:
		return false;
	}
}