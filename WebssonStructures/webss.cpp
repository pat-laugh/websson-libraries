//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "webss.h"

using namespace std;
using namespace webss;

Webss::Webss() : t(WebssType::NONE) {}

Webss::Webss(bool tBool) : t(WebssType::PRIMITIVE_BOOL), tBool(tBool) {}
Webss::Webss(Keyword keyword) : t(WebssType::PRIMITIVE_BOOL)
{
	switch (keyword)
	{
	case Keyword::KEY_NULL:
		t = WebssType::PRIMITIVE_NULL;
		break;
	case Keyword::KEY_FALSE:
		tBool = false;
		break;
	case Keyword::KEY_TRUE:
		tBool = true;
		break;
	default:
		throw runtime_error("can't get keyword value: " + keyword.toString());
	}
}
Webss::Webss(int tInt) : t(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(WebssInt tInt) : t(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(size_t tInt) : t(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(double tDouble) : t(WebssType::PRIMITIVE_DOUBLE), tDouble(tDouble) {}

Webss::Webss(const char* s) : t(WebssType::PRIMITIVE_STRING), tString(new string(s)) {}

#define PATTERN_CONSTRUCT_MOVE(type, name, con) \
Webss::Webss(type&& name) : t(WebssType::con), name(new type(move(name))) {}

PATTERN_CONSTRUCT_MOVE(string, tString, PRIMITIVE_STRING)
PATTERN_CONSTRUCT_MOVE(Document, document, DOCUMENT)
PATTERN_CONSTRUCT_MOVE(Dictionary, dict, DICTIONARY)
PATTERN_CONSTRUCT_MOVE(List, list, LIST)
PATTERN_CONSTRUCT_MOVE(Tuple, tuple, TUPLE)
PATTERN_CONSTRUCT_MOVE(FunctionHeadBinary, fheadBinary, FUNCTION_HEAD_BINARY)
PATTERN_CONSTRUCT_MOVE(FunctionHeadScoped, fheadScoped, FUNCTION_HEAD_SCOPED)
PATTERN_CONSTRUCT_MOVE(FunctionHeadStandard, fheadStandard, FUNCTION_HEAD_STANDARD)
PATTERN_CONSTRUCT_MOVE(FunctionBinary, funcBinary, FUNCTION_BINARY)
PATTERN_CONSTRUCT_MOVE(FunctionScoped, funcScoped, FUNCTION_SCOPED)
PATTERN_CONSTRUCT_MOVE(FunctionStandard, funcStandard, FUNCTION_STANDARD)
PATTERN_CONSTRUCT_MOVE(Namespace, nspace, NAMESPACE)
Webss::Webss(Enum&& name, bool) : t(WebssType::ENUM), nspace(new Namespace(move(name))) {}
PATTERN_CONSTRUCT_MOVE(BlockHead, blockHead, BLOCK_HEAD)
PATTERN_CONSTRUCT_MOVE(Block, block, BLOCK)

#define PATTERN_CONSTRUCT_CONST(type, name, con) \
Webss::Webss(const type& name) : t(WebssType::con), name(new type(name)) {}

PATTERN_CONSTRUCT_CONST(string, tString, PRIMITIVE_STRING)
PATTERN_CONSTRUCT_CONST(Document, document, DOCUMENT)
PATTERN_CONSTRUCT_CONST(Dictionary, dict, DICTIONARY)
PATTERN_CONSTRUCT_CONST(List, list, LIST)
PATTERN_CONSTRUCT_CONST(Tuple, tuple, TUPLE)
PATTERN_CONSTRUCT_CONST(FunctionHeadBinary, fheadBinary, FUNCTION_HEAD_BINARY)
PATTERN_CONSTRUCT_CONST(FunctionHeadScoped, fheadScoped, FUNCTION_HEAD_SCOPED)
PATTERN_CONSTRUCT_CONST(FunctionHeadStandard, fheadStandard, FUNCTION_HEAD_STANDARD)
PATTERN_CONSTRUCT_CONST(FunctionBinary, funcBinary, FUNCTION_BINARY)
PATTERN_CONSTRUCT_CONST(FunctionScoped, funcScoped, FUNCTION_SCOPED)
PATTERN_CONSTRUCT_CONST(FunctionStandard, funcStandard, FUNCTION_STANDARD)
PATTERN_CONSTRUCT_CONST(Namespace, nspace, NAMESPACE)
Webss::Webss(const Enum& name, bool) : t(WebssType::ENUM), nspace(new Namespace(name)) {}
PATTERN_CONSTRUCT_CONST(BlockHead, blockHead, BLOCK_HEAD)
PATTERN_CONSTRUCT_CONST(Block, block, BLOCK)

#define PatternConstructFunction(Type, TypeCaps) \
Webss::Webss(FunctionHead##Type&& head, Webss&& body) \
{ \
	switch (body.t) \
	{ \
	case WebssType::DICTIONARY: \
		func##Type = new Function##Type(move(head), move(*body.dict)); \
		break; \
	case WebssType::LIST: \
		func##Type = new Function##Type(move(head), move(*body.list)); \
		break; \
	case WebssType::TUPLE: \
		func##Type = new Function##Type(move(head), move(*body.tuple)); \
		break; \
	default: \
		throw domain_error(""); \
	} \
	t = WebssType::FUNCTION_##TypeCaps; \
}

PatternConstructFunction(Binary, BINARY)
PatternConstructFunction(Standard, STANDARD)

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
		delete dict;
		break;
	case WebssType::LIST:
		delete list;
		break;
	case WebssType::TUPLE:
		delete tuple;
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		delete fheadBinary;
		break;
	case WebssType::FUNCTION_HEAD_SCOPED:
		delete fheadScoped;
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		delete fheadStandard;
		break;
	case WebssType::FUNCTION_BINARY:
		delete funcBinary;
		break;
	case WebssType::FUNCTION_SCOPED:
		delete funcScoped;
		break;
	case WebssType::FUNCTION_STANDARD:
		delete funcStandard;
		break;
	case WebssType::NAMESPACE: case WebssType::ENUM:
		delete nspace;
		break;
	case WebssType::BLOCK_HEAD:
		delete blockHead;
		break;
	case WebssType::BLOCK:
		delete block;
		break;
	case WebssType::ENTITY:
		ent.~BasicEntity();
		break;
	case WebssType::DEFAULT:
		tDefault.~shared_ptr();
		break;
	default:
		return;
	}
	t = WebssType::NONE;
}

Webss& Webss::operator=(Webss&& o)
{
	destroyUnion();
	copyUnion(move(o));
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
	switch (o.t)
	{
	default:
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
		document= o.document;
		break;
	case WebssType::DICTIONARY:
		dict = o.dict;
		break;
	case WebssType::LIST:
		list = o.list;
		break;
	case WebssType::TUPLE:
		tuple = o.tuple;
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		fheadBinary = o.fheadBinary;
		break;
	case WebssType::FUNCTION_HEAD_SCOPED:
		fheadScoped = o.fheadScoped;
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		fheadStandard = o.fheadStandard;
		break;
	case WebssType::FUNCTION_BINARY:
		funcBinary = o.funcBinary;
		break;
	case WebssType::FUNCTION_SCOPED:
		funcScoped = o.funcScoped;
		break;
	case WebssType::FUNCTION_STANDARD:
		funcStandard = o.funcStandard;
		break;
	case WebssType::NAMESPACE: case WebssType::ENUM:
		nspace = o.nspace;
		break;
	case WebssType::BLOCK_HEAD:
		blockHead = o.blockHead;
		break;
	case WebssType::BLOCK:
		block = o.block;
		break;
	case WebssType::ENTITY:
		new (&ent) Entity(move(o.ent));
		o.ent.~BasicEntity();
		break;
	case WebssType::DEFAULT:
		new (&tDefault) Default(move(o.tDefault));
		o.tDefault.~shared_ptr();
		break;
	}
	t = o.t;
	o.t = WebssType::NONE;
}

void Webss::copyUnion(const Webss& o)
{
	switch (o.t)
	{
	default:
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
	case WebssType::LIST:
		list = new List(*o.list);
		break;
	case WebssType::TUPLE:
		tuple = new Tuple(*o.tuple);
		break;
	case WebssType::FUNCTION_HEAD_BINARY:
		fheadBinary = new FunctionHeadBinary(*o.fheadBinary);
		break;
	case WebssType::FUNCTION_HEAD_SCOPED:
		fheadScoped = new FunctionHeadScoped(*o.fheadScoped);
		break;
	case WebssType::FUNCTION_HEAD_STANDARD:
		fheadStandard = new FunctionHeadStandard(*o.fheadStandard);
		break;
	case WebssType::FUNCTION_BINARY:
		funcBinary = new FunctionBinary(*o.funcBinary);
		break;
	case WebssType::FUNCTION_SCOPED:
		funcScoped = new FunctionScoped(*o.funcScoped);
		break;
	case WebssType::FUNCTION_STANDARD:
		funcStandard = new FunctionStandard(*o.funcStandard);
		break;
	case WebssType::NAMESPACE: case WebssType::ENUM:
		nspace = new Namespace(*o.nspace);
		break;
	case WebssType::BLOCK_HEAD:
		blockHead = new BlockHead(*o.blockHead);
		break;
	case WebssType::BLOCK:
		block = new Block(*o.block);
		break;
	case WebssType::ENTITY:
		new (&ent) Entity(o.ent);
		break;
	case WebssType::DEFAULT:
		new (&tDefault) Default(o.tDefault);
		break;
	}
	t = o.t;
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
	case WebssType::FUNCTION_BINARY:
		return (*funcBinary)[index];
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue()[index];
	case WebssType::FUNCTION_STANDARD:
		return (*funcStandard)[index];
	case WebssType::BLOCK:
		return block->getValue()[index];
	case WebssType::ENTITY:
		return ent.getContent()[index];
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
		return (*dict)[key];
	case WebssType::TUPLE:
		return (*tuple)[key];
	case WebssType::FUNCTION_BINARY:
		return (*funcBinary)[key];
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue()[key];
	case WebssType::FUNCTION_STANDARD:
		return (*funcStandard)[key];
	case WebssType::BLOCK:
		return block->getValue()[key];
	case WebssType::ENTITY:
		return ent.getContent()[key];
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
	case WebssType::FUNCTION_BINARY:
		return funcBinary->at(index);
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().at(index);
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->at(index);
	case WebssType::BLOCK:
		return block->getValue().at(index);
	case WebssType::ENTITY:
		return ent.getContent().at(index);
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
		return dict->at(key);
	case WebssType::TUPLE:
		return tuple->at(key);
	case WebssType::FUNCTION_BINARY:
		return funcBinary->at(key);
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().at(key);
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->at(key);
	case WebssType::BLOCK:
		return block->getValue().at(key);
	case WebssType::ENTITY:
		return ent.getContent().at(key);
	case WebssType::DEFAULT:
		return tDefault->at(key);
	default:
		throw runtime_error(ERROR_ACCESS + t.toString() + ERROR_ACCESS_KEY);
	}
}

bool Webss::hasEntity() const { return t == WebssType::ENTITY; }

const char ERROR_COULD_NOT_GETs1[] = "tried to get ";
const char ERROR_COULD_NOT_GETs2[] = " but webss type was ";
#define PATTERN_GET_CONST(x, y, z) \
switch (t) \
{ \
	case WebssType::ENTITY: \
		return ent.getContent().y; \
	case WebssType::DEFAULT: \
		return tDefault->y; \
	case WebssType::NAMESPACE: \
		return nspace->at(nspace->getName()).getContent().y; \
	case z: \
		return x; \
	default: \
		throw runtime_error(ERROR_COULD_NOT_GETs1 + WebssType(z).toString() + ERROR_COULD_NOT_GETs2 + t.toString()); \
}

WebssType Webss::getType() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().getType();
	case WebssType::DEFAULT:
		return tDefault->getType();
	case WebssType::NAMESPACE:
		return nspace->has(nspace->getName()) ? (*nspace)[nspace->getName()].getContent().getType() : t;
	case WebssType::FUNCTION_BINARY:
		return funcBinary->getType();
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().getType();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->getType();
	case WebssType::BLOCK:
		return block->getValue().getType();
	default:
		return t;
	}
}

bool Webss::getBool() const { PATTERN_GET_CONST(tBool, getBool(), WebssType::PRIMITIVE_BOOL); }
double Webss::getDouble() const { PATTERN_GET_CONST(tDouble, getDouble(), WebssType::PRIMITIVE_DOUBLE); }
const std::string& Webss::getString() const { PATTERN_GET_CONST(*tString, getString(), WebssType::PRIMITIVE_STRING); }
const Document& Webss::getDocument() const { PATTERN_GET_CONST(*document, getDocument(), WebssType::DOCUMENT); }
const FunctionHeadBinary& Webss::getFunctionHeadBinary() const { PATTERN_GET_CONST(*fheadBinary, getFunctionHeadBinary(), WebssType::FUNCTION_HEAD_BINARY); }
const FunctionHeadScoped& Webss::getFunctionHeadScoped() const { PATTERN_GET_CONST(*fheadScoped, getFunctionHeadScoped(), WebssType::FUNCTION_HEAD_SCOPED); }
const FunctionHeadStandard& Webss::getFunctionHeadStandard() const { PATTERN_GET_CONST(*fheadStandard, getFunctionHeadStandard(), WebssType::FUNCTION_HEAD_STANDARD); }
const FunctionBinary& Webss::getFunctionBinary() const { PATTERN_GET_CONST(*funcBinary, getFunctionBinary(), WebssType::FUNCTION_BINARY); }
const FunctionScoped& Webss::getFunctionScoped() const { PATTERN_GET_CONST(*funcScoped, getFunctionScoped(), WebssType::FUNCTION_SCOPED); }
const FunctionStandard& Webss::getFunctionStandard() const { PATTERN_GET_CONST(*funcStandard, getFunctionStandard(), WebssType::FUNCTION_STANDARD); }
const Enum& Webss::getEnum() const { PATTERN_GET_CONST(*nspace, getEnum(), WebssType::ENUM); }
const BlockHead& Webss::getBlockHead() const { PATTERN_GET_CONST(*blockHead, getBlockHead(), WebssType::BLOCK_HEAD); }
const Block& Webss::getBlock() const { PATTERN_GET_CONST(*block, getBlock(), WebssType::BLOCK); }

WebssInt Webss::getInt() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().getInt();
	case WebssType::DEFAULT:
		return tDefault->getInt();
	case WebssType::NAMESPACE:
		return nspace->at(nspace->getName()).getContent().getInt();
	case WebssType::PRIMITIVE_INT:
		return tInt;
	default:
		throw runtime_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::PRIMITIVE_INT).toString() + ERROR_COULD_NOT_GETs2 + t.toString());
	}
}

const Dictionary& Webss::getDictionary() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().getDictionary();
	case WebssType::DEFAULT:
		return tDefault->getDictionary();
	case WebssType::NAMESPACE:
		return nspace->at(nspace->getName()).getContent().getDictionary();
	case WebssType::DICTIONARY:
		return *dict;
	case WebssType::FUNCTION_BINARY:
		return funcBinary->getDictionary();
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().getDictionary();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->getDictionary();
	case WebssType::BLOCK:
		return block->getValue().getDictionary();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::LIST).toString() + ERROR_COULD_NOT_GETs2 + t.toString());

	}
}
const List& Webss::getList() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().getList();
	case WebssType::DEFAULT:
		return tDefault->getList();
	case WebssType::NAMESPACE:
		return nspace->at(nspace->getName()).getContent().getList();
	case WebssType::LIST:
		return *list;
	case WebssType::FUNCTION_BINARY:
		return funcBinary->getList();
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().getList();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->getList();
	case WebssType::BLOCK:
		return block->getValue().getList();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::LIST).toString() + ERROR_COULD_NOT_GETs2 + t.toString());

	}
}
const Tuple& Webss::getTuple() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().getTuple();
	case WebssType::DEFAULT:
		return tDefault->getTuple();
	case WebssType::NAMESPACE:
		return nspace->at(nspace->getName()).getContent().getTuple();
	case WebssType::TUPLE:
		return *tuple;
	case WebssType::FUNCTION_BINARY:
		return funcBinary->getTuple();
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().getTuple();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->getTuple();
	case WebssType::BLOCK:
		return block->getValue().getTuple();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::TUPLE).toString() + ERROR_COULD_NOT_GETs2 + t.toString());
	}
}

const Namespace& Webss::getNamespace() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().getNamespace();
	case WebssType::DEFAULT:
		return tDefault->getNamespace();
	case WebssType::NAMESPACE:
		return *nspace;
	default:
		throw runtime_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::NAMESPACE).toString() + ERROR_COULD_NOT_GETs2 + t.toString()); \
	}
}

#define PATTERN_IS(x, y) \
switch (t) \
{ \
	case WebssType::ENTITY: \
		return ent.getContent().y; \
	case WebssType::DEFAULT: \
		return tDefault->y; \
	case WebssType::NAMESPACE: \
		return nspace->has(nspace->getName()) && (*nspace)[nspace->getName()].getContent().y; \
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
bool Webss::isDocument() const { PATTERN_IS(WebssType::DOCUMENT, isDocument()) }
bool Webss::isFunctionHeadBinary() const { PATTERN_IS(WebssType::FUNCTION_HEAD_BINARY, isFunctionHeadBinary()) }
bool Webss::isFunctionHeadScoped() const { PATTERN_IS(WebssType::FUNCTION_HEAD_SCOPED, isFunctionHeadScoped()) }
bool Webss::isFunctionHeadStandard() const { PATTERN_IS(WebssType::FUNCTION_HEAD_STANDARD, isFunctionHeadStandard()) }
bool Webss::isEnum() const { PATTERN_IS(WebssType::ENUM, isEnum()) }
bool Webss::isBlockHead() const { PATTERN_IS(WebssType::BLOCK_HEAD, isBlockHead()) }
bool Webss::isBlock() const { PATTERN_IS(WebssType::BLOCK, isBlock()) }

bool Webss::isDictionary() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().isDictionary();
	case WebssType::DEFAULT:
		return tDefault->isDictionary();
	case WebssType::DICTIONARY:
		return true;
	case WebssType::FUNCTION_BINARY:
		return funcBinary->isDictionary();
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().isDictionary();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->isDictionary();
	case WebssType::BLOCK:
		return block->getValue().isDictionary();
	default:
		return false;
	}
}
bool Webss::isList() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().isList();
	case WebssType::DEFAULT:
		return tDefault->isList();
	case WebssType::LIST:
		return true;
	case WebssType::FUNCTION_BINARY:
		return funcBinary->isList();
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().isList();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->isList();
	case WebssType::BLOCK:
		return block->getValue().isList();
	default:
		return false;
	}
}
bool Webss::isTuple() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().isTuple();
	case WebssType::DEFAULT:
		return tDefault->isTuple();
	case WebssType::TUPLE:
		return true;
	case WebssType::FUNCTION_BINARY:
		return funcBinary->isTuple();
	case WebssType::FUNCTION_SCOPED:
		return funcScoped->getValue().isTuple();
	case WebssType::FUNCTION_STANDARD:
		return funcStandard->isTuple();
	case WebssType::BLOCK:
		return block->getValue().isTuple();
	default:
		return false;
	}
}
bool Webss::isNamespace() const
{
	switch (t)
	{
	case WebssType::ENTITY:
		return ent.getContent().isNamespace();
	case WebssType::DEFAULT:
		return tDefault->isNamespace();
	case WebssType::NAMESPACE:
		return true;
	default:
		return false;
	}
}

bool Webss::isConcrete() const
{
	switch (t)
	{
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE: case WebssType::PRIMITIVE_STRING:
	case WebssType::DICTIONARY: case WebssType::LIST: case WebssType::TUPLE:
	case WebssType::FUNCTION_BINARY: case WebssType::FUNCTION_SCOPED: case WebssType::FUNCTION_STANDARD: case WebssType::BLOCK:
		return true;
	case WebssType::ENTITY:
		return ent.getContent().isConcrete();
	case WebssType::DEFAULT:
		return tDefault->isConcrete();
	case WebssType::NAMESPACE:
		return nspace->has(nspace->getName()) && (*nspace)[nspace->getName()].getContent().isConcrete();
	default:
		return false;
	}
}