//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "webss.h"

using namespace std;
using namespace webss;

Webss::Webss() {}

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
		throw runtime_error("can't get keyword value: " + keyword.toString());
	}
}
Webss::Webss(int tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(WebssInt tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(size_t tInt) : type(WebssType::PRIMITIVE_INT), tInt(tInt) {}
Webss::Webss(double tDouble) : type(WebssType::PRIMITIVE_DOUBLE), tDouble(tDouble) {}

Webss::Webss(const char* s) : type(WebssType::PRIMITIVE_STRING), tString(new string(s)) {}

#define PATTERN_CONSTRUCT_MOVE(T, name, con) \
Webss::Webss(T&& name) : type(WebssType::con), name(new T(move(name))) {}

PATTERN_CONSTRUCT_MOVE(string, tString, PRIMITIVE_STRING)
PATTERN_CONSTRUCT_MOVE(Document, document, DOCUMENT)
PATTERN_CONSTRUCT_MOVE(Dictionary, dict, DICTIONARY)
PATTERN_CONSTRUCT_MOVE(List, list, LIST)
PATTERN_CONSTRUCT_MOVE(Tuple, tuple, TUPLE)
PATTERN_CONSTRUCT_MOVE(TemplateHeadBinary, fheadBinary, TEMPLATE_HEAD_BINARY)
PATTERN_CONSTRUCT_MOVE(TemplateHeadScoped, fheadScoped, TEMPLATE_HEAD_SCOPED)
PATTERN_CONSTRUCT_MOVE(TemplateHeadStandard, fheadStandard, TEMPLATE_HEAD_STANDARD)
PATTERN_CONSTRUCT_MOVE(TemplateHeadText, fheadText, TEMPLATE_HEAD_TEXT)
PATTERN_CONSTRUCT_MOVE(TemplateBinary, templBinary, TEMPLATE_BINARY)
PATTERN_CONSTRUCT_MOVE(TemplateScoped, templScoped, TEMPLATE_SCOPED)
PATTERN_CONSTRUCT_MOVE(TemplateStandard, templStandard, TEMPLATE_STANDARD)
PATTERN_CONSTRUCT_MOVE(TemplateText, templText, TEMPLATE_TEXT)
PATTERN_CONSTRUCT_MOVE(Namespace, nspace, NAMESPACE)
PATTERN_CONSTRUCT_MOVE(Enum, tEnum, ENUM)
PATTERN_CONSTRUCT_MOVE(BlockHead, blockHead, BLOCK_HEAD)
PATTERN_CONSTRUCT_MOVE(Block, block, BLOCK)

#define PATTERN_CONSTRUCT_CONST(T, name, con) \
Webss::Webss(const T& name) : type(WebssType::con), name(new T(name)) {}

PATTERN_CONSTRUCT_CONST(string, tString, PRIMITIVE_STRING)
PATTERN_CONSTRUCT_CONST(Document, document, DOCUMENT)
PATTERN_CONSTRUCT_CONST(Dictionary, dict, DICTIONARY)
PATTERN_CONSTRUCT_CONST(List, list, LIST)
PATTERN_CONSTRUCT_CONST(Tuple, tuple, TUPLE)
PATTERN_CONSTRUCT_CONST(TemplateHeadBinary, fheadBinary, TEMPLATE_HEAD_BINARY)
PATTERN_CONSTRUCT_CONST(TemplateHeadScoped, fheadScoped, TEMPLATE_HEAD_SCOPED)
PATTERN_CONSTRUCT_CONST(TemplateHeadStandard, fheadStandard, TEMPLATE_HEAD_STANDARD)
PATTERN_CONSTRUCT_CONST(TemplateHeadText, fheadText, TEMPLATE_HEAD_TEXT)
PATTERN_CONSTRUCT_CONST(TemplateBinary, templBinary, TEMPLATE_BINARY)
PATTERN_CONSTRUCT_CONST(TemplateScoped, templScoped, TEMPLATE_SCOPED)
PATTERN_CONSTRUCT_CONST(TemplateStandard, templStandard, TEMPLATE_STANDARD)
PATTERN_CONSTRUCT_CONST(TemplateText, templText, TEMPLATE_TEXT)
PATTERN_CONSTRUCT_CONST(Namespace, nspace, NAMESPACE)
PATTERN_CONSTRUCT_CONST(Enum, tEnum, ENUM)
PATTERN_CONSTRUCT_CONST(BlockHead, blockHead, BLOCK_HEAD)
PATTERN_CONSTRUCT_CONST(Block, block, BLOCK)

Webss::Webss(TemplateHeadSelf) : type(WebssType::TEMPLATE_HEAD_SELF) {}

#define PatternConstructTemplate(Type, TypeCaps) \
Webss::Webss(TemplateHead##Type&& head, Webss&& body) \
{ \
	switch (body.type) \
	{ \
	case WebssType::DICTIONARY: \
		templ##Type = new Template##Type(move(head), move(*body.dict)); \
		break; \
	case WebssType::LIST: \
		templ##Type = new Template##Type(move(head), move(*body.list)); \
		break; \
	case WebssType::TUPLE: \
		templ##Type = new Template##Type(move(head), move(*body.tuple)); \
		break; \
	default: \
		assert(false); throw domain_error(""); \
	} \
	type = WebssType::TEMPLATE_##TypeCaps; \
}

PatternConstructTemplate(Binary, BINARY)
PatternConstructTemplate(Standard, STANDARD)
PatternConstructTemplate(Text, TEXT)

Webss::Webss(Webss&& o) { copyUnion(move(o)); }
Webss::Webss(const Webss& o) { copyUnion(o); }
Webss::~Webss() { destroyUnion(); }

void Webss::destroyUnion()
{
	switch (type)
	{
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		break;
	case WebssType::ENTITY:
		ent.~BasicEntity();
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
	case WebssType::LIST:
		delete list;
		break;
	case WebssType::TUPLE:
		delete tuple;
		break;
	case WebssType::TEMPLATE_HEAD_BINARY:
		delete fheadBinary;
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		delete fheadScoped;
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		delete fheadStandard;
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		delete fheadText;
		break;
	case WebssType::TEMPLATE_BINARY:
		delete templBinary;
		break;
	case WebssType::TEMPLATE_SCOPED:
		delete templScoped;
		break;
	case WebssType::TEMPLATE_STANDARD:
		delete templStandard;
		break;
	case WebssType::TEMPLATE_TEXT:
		delete templText;
		break;
	case WebssType::NAMESPACE:
		delete nspace;
		break;
	case WebssType::ENUM:
		delete tEnum;
		break;
	case WebssType::BLOCK_HEAD:
		delete blockHead;
		break;
	case WebssType::BLOCK:
		delete block;
		break;
	default:
		assert(false); throw domain_error("");
	}
	type = WebssType::NONE;
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
	switch (o.type)
	{
	case WebssType::NONE: case WebssType::PRIMITIVE_NULL:
		break;
	case WebssType::ENTITY:
		new (&ent) Entity(move(o.ent));
		o.ent.~BasicEntity();
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
	case WebssType::TEMPLATE_HEAD_BINARY:
		fheadBinary = o.fheadBinary;
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		fheadScoped = o.fheadScoped;
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		fheadStandard = o.fheadStandard;
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		fheadText = o.fheadText;
		break;
	case WebssType::TEMPLATE_BINARY:
		templBinary = o.templBinary;
		break;
	case WebssType::TEMPLATE_SCOPED:
		templScoped = o.templScoped;
		break;
	case WebssType::TEMPLATE_STANDARD:
		templStandard = o.templStandard;
		break;
	case WebssType::TEMPLATE_TEXT:
		templText = o.templText;
		break;
	case WebssType::NAMESPACE:
		nspace = o.nspace;
		break;
	case WebssType::ENUM:
		tEnum = o.tEnum;
		break;
	case WebssType::BLOCK_HEAD:
		blockHead = o.blockHead;
		break;
	case WebssType::BLOCK:
		block = o.block;
		break;
	default:
		assert(false); throw domain_error("");
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
	case WebssType::LIST:
		list = new List(*o.list);
		break;
	case WebssType::TUPLE:
		tuple = new Tuple(*o.tuple);
		break;
	case WebssType::TEMPLATE_HEAD_BINARY:
		fheadBinary = new TemplateHeadBinary(*o.fheadBinary);
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		fheadScoped = new TemplateHeadScoped(*o.fheadScoped);
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		fheadStandard = new TemplateHeadStandard(*o.fheadStandard);
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		fheadText = new TemplateHeadText(*o.fheadText);
		break;
	case WebssType::TEMPLATE_BINARY:
		templBinary = new TemplateBinary(*o.templBinary);
		break;
	case WebssType::TEMPLATE_SCOPED:
		templScoped = new TemplateScoped(*o.templScoped);
		break;
	case WebssType::TEMPLATE_STANDARD:
		templStandard = new TemplateStandard(*o.templStandard);
		break;
	case WebssType::TEMPLATE_TEXT:
		templText = new TemplateText(*o.templText);
		break;
	case WebssType::NAMESPACE:
		nspace = new Namespace(*o.nspace);
		break;
	case WebssType::ENUM:
		tEnum = new Enum(*o.tEnum);
		break;
	case WebssType::BLOCK_HEAD:
		blockHead = new BlockHead(*o.blockHead);
		break;
	case WebssType::BLOCK:
		block = new Block(*o.block);
		break;
	default:
		assert(false); throw domain_error("");
	}
	type = o.type;
}

const char ERROR_ACCESS[] = "can't access ";
const char ERROR_ACCESS_INDEX[] = " with an index";
const char ERROR_ACCESS_KEY[] = " with a key";

const Webss& Webss::operator[](int index) const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent()[index];
	case WebssType::DEFAULT:
		return (*tDefault)[index];
	case WebssType::LIST:
		return (*list)[index];
	case WebssType::TUPLE:
		return (*tuple)[index];
	case WebssType::TEMPLATE_BINARY:
		return (*templBinary)[index];
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue()[index];
	case WebssType::TEMPLATE_STANDARD:
		return (*templStandard)[index];
	case WebssType::TEMPLATE_TEXT:
		return (*templText)[index];
	case WebssType::BLOCK:
		return block->getValue()[index];
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_INDEX);
	}
}

const Webss& Webss::operator[](const std::string& key) const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent()[key];
	case WebssType::DEFAULT:
		return (*tDefault)[key];
	case WebssType::DICTIONARY:
		return (*dict)[key];
	case WebssType::TUPLE:
		return (*tuple)[key];
	case WebssType::TEMPLATE_BINARY:
		return (*templBinary)[key];
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue()[key];
	case WebssType::TEMPLATE_STANDARD:
		return (*templStandard)[key];
	case WebssType::TEMPLATE_TEXT:
		return (*templText)[key];
	case WebssType::BLOCK:
		return block->getValue()[key];
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_KEY);
	}
}

const Webss& Webss::at(int index) const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().at(index);
	case WebssType::DEFAULT:
		return tDefault->at(index);
	case WebssType::LIST:
		return list->at(index);
	case WebssType::TUPLE:
		return tuple->at(index);
	case WebssType::TEMPLATE_BINARY:
		return templBinary->at(index);
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().at(index);
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->at(index);
	case WebssType::TEMPLATE_TEXT:
		return templText->at(index);
	case WebssType::BLOCK:
		return block->getValue().at(index);
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_INDEX);
	}
}

const Webss& Webss::at(const std::string& key) const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().at(key);
	case WebssType::DEFAULT:
		return tDefault->at(key);
	case WebssType::DICTIONARY:
		return dict->at(key);
	case WebssType::TUPLE:
		return tuple->at(key);
	case WebssType::TEMPLATE_BINARY:
		return templBinary->at(key);
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().at(key);
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->at(key);
	case WebssType::TEMPLATE_TEXT:
		return templText->at(key);
	case WebssType::BLOCK:
		return block->getValue().at(key);
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_KEY);
	}
}

WebssType Webss::getType() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().getType();
	case WebssType::DEFAULT:
		return tDefault->getType();
	case WebssType::TEMPLATE_BINARY:
		return templBinary->getType();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().getType();
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->getType();
	case WebssType::TEMPLATE_TEXT:
		return templText->getType();
	case WebssType::BLOCK:
		return block->getValue().getType();
	default:
		return type;
	}
}

const char ERROR_COULD_NOT_GETs1[] = "tried to get ";
const char ERROR_COULD_NOT_GETs2[] = " but webss type was ";
#define PATTERN_GET_CONST(x, y, z) \
switch (type) \
{ \
	case WebssType::ENTITY: \
		return ent.getContent().y; \
	case WebssType::DEFAULT: \
		return tDefault->y; \
	case z: \
		return x; \
	default: \
		throw runtime_error(ERROR_COULD_NOT_GETs1 + WebssType(z).toString() + ERROR_COULD_NOT_GETs2 + type.toString()); \
}

bool Webss::getBool() const { PATTERN_GET_CONST(tBool, getBool(), WebssType::PRIMITIVE_BOOL); }
WebssInt Webss::getInt() const { PATTERN_GET_CONST(tInt, getInt(), WebssType::PRIMITIVE_INT); }
double Webss::getDouble() const { PATTERN_GET_CONST(tDouble, getDouble(), WebssType::PRIMITIVE_DOUBLE); }
const std::string& Webss::getString() const { PATTERN_GET_CONST(*tString, getString(), WebssType::PRIMITIVE_STRING); }
const Document& Webss::getDocument() const { PATTERN_GET_CONST(*document, getDocument(), WebssType::DOCUMENT); }
const TemplateHeadBinary& Webss::getTemplateHeadBinary() const { PATTERN_GET_CONST(*fheadBinary, getTemplateHeadBinary(), WebssType::TEMPLATE_HEAD_BINARY); }
const TemplateHeadScoped& Webss::getTemplateHeadScoped() const { PATTERN_GET_CONST(*fheadScoped, getTemplateHeadScoped(), WebssType::TEMPLATE_HEAD_SCOPED); }
const TemplateHeadStandard& Webss::getTemplateHeadStandard() const { PATTERN_GET_CONST(*fheadStandard, getTemplateHeadStandard(), WebssType::TEMPLATE_HEAD_STANDARD); }
const TemplateHeadText& Webss::getTemplateHeadText() const { PATTERN_GET_CONST(*fheadText, getTemplateHeadText(), WebssType::TEMPLATE_HEAD_TEXT); }
const TemplateBinary& Webss::getTemplateBinary() const { PATTERN_GET_CONST(*templBinary, getTemplateBinary(), WebssType::TEMPLATE_BINARY); }
const TemplateScoped& Webss::getTemplateScoped() const { PATTERN_GET_CONST(*templScoped, getTemplateScoped(), WebssType::TEMPLATE_SCOPED); }
const TemplateStandard& Webss::getTemplateStandard() const { PATTERN_GET_CONST(*templStandard, getTemplateStandard(), WebssType::TEMPLATE_STANDARD); }
const TemplateText& Webss::getTemplateText() const { PATTERN_GET_CONST(*templText, getTemplateText(), WebssType::TEMPLATE_TEXT); }
const Namespace& Webss::getNamespace() const { PATTERN_GET_CONST(*nspace, getNamespace(), WebssType::NAMESPACE); }
const Enum& Webss::getEnum() const { PATTERN_GET_CONST(*tEnum, getEnum(), WebssType::ENUM); }
const BlockHead& Webss::getBlockHead() const { PATTERN_GET_CONST(*blockHead, getBlockHead(), WebssType::BLOCK_HEAD); }
const Block& Webss::getBlock() const { PATTERN_GET_CONST(*block, getBlock(), WebssType::BLOCK); }

const Dictionary& Webss::getDictionary() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().getDictionary();
	case WebssType::DEFAULT:
		return tDefault->getDictionary();
	case WebssType::DICTIONARY:
		return *dict;
	case WebssType::TEMPLATE_BINARY:
		return templBinary->getDictionary();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().getDictionary();
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->getDictionary();
	case WebssType::TEMPLATE_TEXT:
		return templText->getDictionary();
	case WebssType::BLOCK:
		return block->getValue().getDictionary();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::LIST).toString() + ERROR_COULD_NOT_GETs2 + type.toString());

	}
}
const List& Webss::getList() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().getList();
	case WebssType::DEFAULT:
		return tDefault->getList();
	case WebssType::LIST:
		return *list;
	case WebssType::TEMPLATE_BINARY:
		return templBinary->getList();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().getList();
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->getList();
	case WebssType::TEMPLATE_TEXT:
		return templText->getList();
	case WebssType::BLOCK:
		return block->getValue().getList();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::LIST).toString() + ERROR_COULD_NOT_GETs2 + type.toString());

	}
}
const Tuple& Webss::getTuple() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().getTuple();
	case WebssType::DEFAULT:
		return tDefault->getTuple();
	case WebssType::TUPLE:
		return *tuple;
	case WebssType::TEMPLATE_BINARY:
		return templBinary->getTuple();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().getTuple();
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->getTuple();
	case WebssType::TEMPLATE_TEXT:
		return templText->getTuple();
	case WebssType::BLOCK:
		return block->getValue().getTuple();
	default:
		throw logic_error(ERROR_COULD_NOT_GETs1 + WebssType(WebssType::TUPLE).toString() + ERROR_COULD_NOT_GETs2 + type.toString());
	}
}

#define PATTERN_IS(x, y) \
switch (type) \
{ \
	case WebssType::ENTITY: \
		return ent.getContent().y; \
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
bool Webss::isDocument() const { PATTERN_IS(WebssType::DOCUMENT, isDocument()) }
bool Webss::isTemplateHeadBinary() const { PATTERN_IS(WebssType::TEMPLATE_HEAD_BINARY, isTemplateHeadBinary()) }
bool Webss::isTemplateHeadScoped() const { PATTERN_IS(WebssType::TEMPLATE_HEAD_SCOPED, isTemplateHeadScoped()) }
bool Webss::isTemplateHeadStandard() const { PATTERN_IS(WebssType::TEMPLATE_HEAD_STANDARD, isTemplateHeadStandard()) }
bool Webss::isTemplateHeadText() const { PATTERN_IS(WebssType::TEMPLATE_HEAD_TEXT, isTemplateHeadText()) }
bool Webss::isNamespace() const { PATTERN_IS(WebssType::NAMESPACE, isNamespace()) }
bool Webss::isEnum() const { PATTERN_IS(WebssType::ENUM, isEnum()) }
bool Webss::isBlockHead() const { PATTERN_IS(WebssType::BLOCK_HEAD, isBlockHead()) }
bool Webss::isBlock() const { PATTERN_IS(WebssType::BLOCK, isBlock()) }

bool Webss::isDictionary() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().isDictionary();
	case WebssType::DEFAULT:
		return tDefault->isDictionary();
	case WebssType::DICTIONARY:
		return true;
	case WebssType::TEMPLATE_BINARY:
		return templBinary->isDictionary();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().isDictionary();
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->isDictionary();
	case WebssType::TEMPLATE_TEXT:
		return templText->isDictionary();
	case WebssType::BLOCK:
		return block->getValue().isDictionary();
	default:
		return false;
	}
}
bool Webss::isList() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().isList();
	case WebssType::DEFAULT:
		return tDefault->isList();
	case WebssType::LIST:
		return true;
	case WebssType::TEMPLATE_BINARY:
		return templBinary->isList();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().isList();
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->isList();
	case WebssType::TEMPLATE_TEXT:
		return templText->isList();
	case WebssType::BLOCK:
		return block->getValue().isList();
	default:
		return false;
	}
}
bool Webss::isTuple() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().isTuple();
	case WebssType::DEFAULT:
		return tDefault->isTuple();
	case WebssType::TUPLE:
		return true;
	case WebssType::TEMPLATE_BINARY:
		return templBinary->isTuple();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().isTuple();
	case WebssType::TEMPLATE_STANDARD:
		return templStandard->isTuple();
	case WebssType::TEMPLATE_TEXT:
		return templText->isTuple();
	case WebssType::BLOCK:
		return block->getValue().isTuple();
	default:
		return false;
	}
}

bool Webss::isAbstract() const
{
	switch (type)
	{
	case WebssType::NONE:
		assert(false); throw domain_error("");
	case WebssType::ENTITY:
		return ent.getContent().isAbstract();
	case WebssType::DEFAULT:
		return tDefault->isAbstract();
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_SCOPED: case WebssType::TEMPLATE_HEAD_SELF: case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
	case WebssType::NAMESPACE: case WebssType::ENUM: case WebssType::BLOCK_HEAD:
		return true;
	default:
		return false;
	}
}

bool Webss::isConcrete() const
{
	return !isAbstract();
}