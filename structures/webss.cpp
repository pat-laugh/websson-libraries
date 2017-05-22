//MIT License
//Copyright 2017 Patrick Laughrea
#include "webss.hpp"

#include <cassert>

#include "dictionary.hpp"
#include "list.hpp"
#include "tuple.hpp"
#include "block.hpp"
#include "template.hpp"
#include "templateScoped.hpp"
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
Webss::Webss(const Default& tDefault) : type(WebssType::DEFAULT), tDefault(tDefault) {}
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
Webss::Webss(List list) : type(WebssType::LIST), list(new List(move(list))) {}
Webss::Webss(List list, bool) : type(WebssType::LIST_TEXT), list(new List(move(list))) {}
Webss::Webss(Tuple tuple) : type(WebssType::TUPLE), tuple(new Tuple(move(tuple))) {}
Webss::Webss(Tuple tuple, bool) : type(WebssType::TUPLE_TEXT), tuple(new Tuple(move(tuple))) {}
Webss::Webss(TemplateHeadBinary theadBinary) : type(WebssType::TEMPLATE_HEAD_BINARY), theadBinary(new TemplateHeadBinary(move(theadBinary))) {}
Webss::Webss(TemplateHeadScoped theadScoped) : type(WebssType::TEMPLATE_HEAD_SCOPED), theadScoped(new TemplateHeadScoped(move(theadScoped))) {}
Webss::Webss(TemplateHeadStandard theadStandard) : type(WebssType::TEMPLATE_HEAD_STANDARD), theadStandard(new TemplateHeadStandard(move(theadStandard))) {}
Webss::Webss(TemplateHeadStandard theadStandard, bool) : type(WebssType::TEMPLATE_HEAD_TEXT), theadStandard(new TemplateHeadStandard(move(theadStandard))) {}
Webss::Webss(TemplateBinary templBinary) : type(WebssType::TEMPLATE_BINARY), templBinary(new TemplateBinary(move(templBinary))) {}
Webss::Webss(TemplateScoped templScoped) : type(WebssType::TEMPLATE_SCOPED), templScoped(new TemplateScoped(move(templScoped))) {}
Webss::Webss(TemplateStandard templStandard) : type(WebssType::TEMPLATE_STANDARD), templStandard(new TemplateStandard(move(templStandard))) {}
Webss::Webss(TemplateStandard templStandard, bool) : type(WebssType::TEMPLATE_TEXT), templStandard(new TemplateStandard(move(templStandard))) {}
Webss::Webss(BlockHead bhead) : type(WebssType::BLOCK_HEAD), bhead(new BlockHead(move(bhead))) {}
Webss::Webss(Block block) : type(WebssType::BLOCK), block(new Block(move(block))) {}
Webss::Webss(Dictionary dict, bool, bool) : type(WebssType::DICTIONARY_ABSTRACT), dict(new Dictionary(move(dict))) {}
Webss::Webss(List list, bool, bool) : type(WebssType::LIST_ABSTRACT), list(new List(move(list))) {}
Webss::Webss(Tuple tuple, bool, bool) : type(WebssType::TUPLE_ABSTRACT), tuple(new Tuple(move(tuple))) {}
Webss::Webss(List list, bool, bool, bool) : type(WebssType::LIST_TEXT_ABSTRACT), list(new List(move(list))) {}
Webss::Webss(Tuple tuple, bool, bool, bool) : type(WebssType::TUPLE_TEXT_ABSTRACT), tuple(new Tuple(move(tuple))) {}

Webss::Webss(TemplateHeadSelf) : type(WebssType::TEMPLATE_HEAD_SELF) {}

Webss::Webss(TemplateHeadBinary&& head, Webss&& body)
{
	switch (body.type)
	{
	case WebssType::DICTIONARY:
		templBinary = new TemplateBinary(move(head), move(*body.dict));
		break;
	case WebssType::LIST:
		templBinary = new TemplateBinary(move(head), move(*body.list));
		break;
	case WebssType::TUPLE:
		templBinary = new TemplateBinary(move(head), move(*body.tuple));
		break;
	case WebssType::TUPLE_TEXT:
		templBinary = new TemplateBinary(move(head), move(*body.tuple), true);
		break;
	default:
		assert(false);
	}
	type = WebssType::TEMPLATE_BINARY;
}

Webss::Webss(TemplateHeadStandard&& head, Webss&& body)
{
	switch (body.type)
	{
	case WebssType::DICTIONARY:
		templStandard = new TemplateStandard(move(head), move(*body.dict));
		break;
	case WebssType::LIST:
		templStandard = new TemplateStandard(move(head), move(*body.list));
		break;
	case WebssType::TUPLE:
		templStandard = new TemplateStandard(move(head), move(*body.tuple));
		break;
	case WebssType::TUPLE_TEXT:
		templStandard = new TemplateStandard(move(head), move(*body.tuple), true);
		break;
	default:
		assert(false);
	}
	type = WebssType::TEMPLATE_STANDARD;
}

Webss::Webss(TemplateHeadStandard&& head, Webss&& body, bool)
{
	switch (body.type)
	{
	case WebssType::DICTIONARY:
		templStandard = new TemplateStandard(move(head), move(*body.dict));
		break;
	case WebssType::LIST:
		templStandard = new TemplateStandard(move(head), move(*body.list));
		break;
	case WebssType::TUPLE:
		templStandard = new TemplateStandard(move(head), move(*body.tuple));
		break;
	case WebssType::TUPLE_TEXT:
		templStandard = new TemplateStandard(move(head), move(*body.tuple), true);
		break;
	default:
		assert(false);
	}
	type = WebssType::TEMPLATE_TEXT;
}

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
	case WebssType::DICTIONARY: case WebssType::DICTIONARY_ABSTRACT:
		delete dict;
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT: case WebssType::LIST_ABSTRACT: case WebssType::LIST_TEXT_ABSTRACT:
		delete list;
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT: case WebssType::TUPLE_ABSTRACT: case WebssType::TUPLE_TEXT_ABSTRACT:
		delete tuple;
		break;
	case WebssType::TEMPLATE_HEAD_BINARY:
		delete theadBinary;
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		delete theadScoped;
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		delete theadStandard;
		break;
	case WebssType::TEMPLATE_BINARY:
		delete templBinary;
		break;
	case WebssType::TEMPLATE_SCOPED:
		delete templScoped;
		break;
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		delete templStandard;
		break;
	case WebssType::NAMESPACE:
		nspace.~Namespace();
		break;
	case WebssType::ENUM:
		tEnum.~Enum();
		break;
	case WebssType::BLOCK_HEAD:
		delete bhead;
		break;
	case WebssType::BLOCK:
		delete block;
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
	case WebssType::DICTIONARY: case WebssType::DICTIONARY_ABSTRACT:
		dict = o.dict;
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT: case WebssType::LIST_ABSTRACT: case WebssType::LIST_TEXT_ABSTRACT:
		list = o.list;
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT: case WebssType::TUPLE_ABSTRACT: case WebssType::TUPLE_TEXT_ABSTRACT:
		tuple = o.tuple;
		break;
	case WebssType::TEMPLATE_HEAD_BINARY:
		theadBinary = o.theadBinary;
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		theadScoped = o.theadScoped;
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		theadStandard = o.theadStandard;
		break;
	case WebssType::TEMPLATE_BINARY:
		templBinary = o.templBinary;
		break;
	case WebssType::TEMPLATE_SCOPED:
		templScoped = o.templScoped;
		break;
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		templStandard = o.templStandard;
		break;
	case WebssType::NAMESPACE:
		new (&nspace) Namespace(move(o.nspace));
		o.nspace.~Namespace();
		break;
	case WebssType::ENUM:
		new (&tEnum) Enum(move(o.tEnum));
		o.tEnum.~Enum();
		break;
	case WebssType::BLOCK_HEAD:
		bhead = o.bhead;
		break;
	case WebssType::BLOCK:
		block = o.block;
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
	case WebssType::DICTIONARY: case WebssType::DICTIONARY_ABSTRACT:
		dict = new Dictionary(*o.dict);
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT: case WebssType::LIST_ABSTRACT: case WebssType::LIST_TEXT_ABSTRACT:
		list = new List(*o.list);
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT: case WebssType::TUPLE_ABSTRACT: case WebssType::TUPLE_TEXT_ABSTRACT:
		tuple = new Tuple(*o.tuple);
		break;
	case WebssType::TEMPLATE_HEAD_BINARY:
		theadBinary = new TemplateHeadBinary(*o.theadBinary);
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		theadScoped = new TemplateHeadScoped(*o.theadScoped);
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
		theadStandard = new TemplateHeadStandard(*o.theadStandard);
		break;
	case WebssType::TEMPLATE_BINARY:
		templBinary = new TemplateBinary(*o.templBinary);
		break;
	case WebssType::TEMPLATE_SCOPED:
		templScoped = new TemplateScoped(*o.templScoped);
		break;
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		templStandard = new TemplateStandard(*o.templStandard);
		break;
	case WebssType::NAMESPACE:
		new (&nspace) Namespace(o.nspace);
		break;
	case WebssType::ENUM:
		new (&tEnum) Enum(o.tEnum);
		break;
	case WebssType::BLOCK_HEAD:
		bhead = new BlockHead(*o.bhead);
		break;
	case WebssType::BLOCK:
		block = new Block(*o.block);
		break;
	default:
		assert(false);
	}
	type = o.type;
}

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
	case WebssType::TEMPLATE_BINARY:
		return (*templBinary)[index];
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue()[index];
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		return (*templStandard)[index];
	case WebssType::BLOCK:
		return block->getValue()[index];
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
	case WebssType::TEMPLATE_BINARY:
		return (*templBinary)[key];
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue()[key];
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		return (*templStandard)[key];
	case WebssType::BLOCK:
		return block->getValue()[key];
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
	case WebssType::TEMPLATE_BINARY:
		return templBinary->at(index);
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().at(index);
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		return templStandard->at(index);
	case WebssType::BLOCK:
		return block->getValue().at(index);
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
	case WebssType::TEMPLATE_BINARY:
		return templBinary->at(key);
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().at(key);
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		return templStandard->at(key);
	case WebssType::BLOCK:
		return block->getValue().at(key);
	default:
		throw runtime_error(ERROR_ACCESS + type.toString() + ERROR_ACCESS_KEY);
	}
}

const Webss& Webss::getWebssLast() const
{
	switch (type)
	{
	case WebssType::ENTITY:
		return ent.getContent().getWebssLast();
	case WebssType::DEFAULT:
		return tDefault->getWebssLast();
	case WebssType::TEMPLATE_BINARY:
		return templBinary->getWebss();
	case WebssType::TEMPLATE_SCOPED:
		return templScoped->getValue().getWebssLast();
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		return templStandard->getWebss();
	case WebssType::BLOCK:
		return block->getValue().getWebssLast();
	default:
		return *this;
	}
}

WebssType Webss::getType() const
{
	return getWebssLast().getTypeRaw();
}

#define PATTERN_GET_CONST_SAFE(Type, Func) \
const auto& webss = getWebssLast(); \
if (webss.getTypeRaw() == Type) \
	return webss.Func(); \
else \
	throw runtime_error("could not get " + WebssType(Type).toString() + "; instead webss type was " + WebssType(webss.getTypeRaw()).toString());

bool Webss::getBool() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_BOOL, getBoolRaw); }
WebssInt Webss::getInt() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_INT, getIntRaw); }
double Webss::getDouble() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_DOUBLE, getDoubleRaw); }
const std::string& Webss::getString() const { PATTERN_GET_CONST_SAFE(WebssType::PRIMITIVE_STRING, getStringRaw); }
const Document& Webss::getDocument() const { PATTERN_GET_CONST_SAFE(WebssType::DOCUMENT, getDocumentRaw); }
const Dictionary& Webss::getDictionary() const { PATTERN_GET_CONST_SAFE(WebssType::DICTIONARY, getDictionaryRaw); }
const TemplateHeadBinary& Webss::getTemplateHeadBinary() const { PATTERN_GET_CONST_SAFE(WebssType::TEMPLATE_HEAD_BINARY, getTemplateHeadBinaryRaw); }
const TemplateHeadScoped& Webss::getTemplateHeadScoped() const { PATTERN_GET_CONST_SAFE(WebssType::TEMPLATE_HEAD_SCOPED, getTemplateHeadScopedRaw); }
const TemplateBinary& Webss::getTemplateBinary() const { PATTERN_GET_CONST_SAFE(WebssType::TEMPLATE_BINARY, getTemplateBinaryRaw); }
const TemplateScoped& Webss::getTemplateScoped() const { PATTERN_GET_CONST_SAFE(WebssType::TEMPLATE_SCOPED, getTemplateScopedRaw); }
const TemplateStandard& Webss::getTemplateStandard() const { PATTERN_GET_CONST_SAFE(WebssType::TEMPLATE_STANDARD, getTemplateStandardRaw); }
const Namespace& Webss::getNamespace() const { PATTERN_GET_CONST_SAFE(WebssType::NAMESPACE, getNamespaceRaw); }
const Enum& Webss::getEnum() const { PATTERN_GET_CONST_SAFE(WebssType::ENUM, getEnumRaw); }
const BlockHead& Webss::getBlockHead() const { PATTERN_GET_CONST_SAFE(WebssType::BLOCK_HEAD, getBlockHeadRaw); }
const Block& Webss::getBlock() const { PATTERN_GET_CONST_SAFE(WebssType::BLOCK, getBlockRaw); }

const List& Webss::getList() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::LIST || type == WebssType::LIST_TEXT || type == WebssType::LIST_ABSTRACT || type == WebssType::LIST_TEXT_ABSTRACT)
		return webss.getListRaw();
	else
		throw runtime_error("could not get " + WebssType(WebssType::LIST).toString() + "; instead webss type was " + WebssType(type).toString());
}
const Tuple& Webss::getTuple() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT || type == WebssType::TUPLE_ABSTRACT || type == WebssType::TUPLE_TEXT_ABSTRACT)
		return webss.getTupleRaw();
	else
		throw runtime_error("could not get " + WebssType(WebssType::TUPLE).toString() + "; instead webss type was " + WebssType(type).toString());
}

const TemplateHeadStandard& Webss::getTemplateHeadStandard() const
{
	const auto& webss = getWebssLast();
	const auto type = webss.getTypeRaw();
	if (type == WebssType::TEMPLATE_HEAD_STANDARD || type == WebssType::TEMPLATE_HEAD_TEXT)
		return webss.getTemplateHeadStandardRaw();
	else
		throw runtime_error("could not get " + WebssType(WebssType::TEMPLATE_HEAD_STANDARD).toString() + "; instead webss type was " + WebssType(type).toString());
}

bool Webss::isNone() const { return getType() == WebssType::NONE; }
bool Webss::isNull() const { return getType() == WebssType::PRIMITIVE_NULL; }
bool Webss::isBool() const { return getType() == WebssType::PRIMITIVE_BOOL; }
bool Webss::isInt() const { return getType() == WebssType::PRIMITIVE_INT; }
bool Webss::isDouble() const { return getType() == WebssType::PRIMITIVE_DOUBLE; }
bool Webss::isString() const { return getType() == WebssType::PRIMITIVE_STRING; }
bool Webss::isDocument() const { return getType() == WebssType::DOCUMENT; }
bool Webss::isDictionary() const { return getType() == WebssType::DICTIONARY; }
bool Webss::isTemplateHeadBinary() const { return getType() == WebssType::TEMPLATE_HEAD_BINARY; }
bool Webss::isTemplateHeadScoped() const { return getType() == WebssType::TEMPLATE_HEAD_SCOPED; }
bool Webss::isNamespace() const { return getType() == WebssType::NAMESPACE; }
bool Webss::isEnum() const { return getType() == WebssType::ENUM; }
bool Webss::isBlockHead() const { return getType() == WebssType::BLOCK_HEAD; }
bool Webss::isBlock() const { return getType() == WebssType::BLOCK; }

bool Webss::isDictionaryAbstract() const { return getType() == WebssType::DICTIONARY_ABSTRACT; }
bool Webss::isListAbstract() const { return getType() == WebssType::LIST_ABSTRACT; }
bool Webss::isTupleAbstract() const { return getType() == WebssType::TUPLE_ABSTRACT; }
bool Webss::isListTextAbstract() const { return getType() == WebssType::LIST_TEXT_ABSTRACT; }
bool Webss::isTupleTextAbstract() const { return getType() == WebssType::TUPLE_TEXT_ABSTRACT; }

bool Webss::isListText() const { return getType() == WebssType::LIST_TEXT; }
bool Webss::isTupleText() const { return getType() == WebssType::TUPLE_TEXT; }
bool Webss::isTemplateHeadText() const { return getType() == WebssType::TEMPLATE_HEAD_TEXT; }

bool Webss::isList() const
{
	const auto type = getType();
	return type == WebssType::LIST || type == WebssType::LIST_TEXT || type == WebssType::LIST_ABSTRACT || type == WebssType::LIST_TEXT_ABSTRACT;
}
bool Webss::isTuple() const
{
	const auto type = getType();
	return type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT || type == WebssType::TUPLE_ABSTRACT || type == WebssType::TUPLE_TEXT_ABSTRACT;
}

bool Webss::isTemplateHeadStandard() const
{
	const auto type = getType();
	return type == WebssType::TEMPLATE_HEAD_STANDARD || type == WebssType::TEMPLATE_HEAD_TEXT;
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
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_SCOPED: case WebssType::TEMPLATE_HEAD_SELF: case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT:
	case WebssType::NAMESPACE: case WebssType::ENUM: case WebssType::BLOCK_HEAD:
	case WebssType::DICTIONARY_ABSTRACT: case WebssType::LIST_ABSTRACT: case WebssType::TUPLE_ABSTRACT: case WebssType::LIST_TEXT_ABSTRACT: case WebssType::TUPLE_TEXT_ABSTRACT:
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
const Tuple& Webss::getTupleRaw() const { assert(getTypeRaw() == WebssType::TUPLE || getTypeRaw() == WebssType::TUPLE_TEXT || getTypeRaw() == WebssType::TUPLE_ABSTRACT); return *tuple; }
const TemplateHeadBinary& Webss::getTemplateHeadBinaryRaw() const { assert(getTypeRaw() == WebssType::TEMPLATE_HEAD_BINARY); return *theadBinary; }
const TemplateHeadScoped& Webss::getTemplateHeadScopedRaw() const { assert(getTypeRaw() == WebssType::TEMPLATE_HEAD_SCOPED); return *theadScoped; }
const TemplateHeadStandard& Webss::getTemplateHeadStandardRaw() const { assert(getTypeRaw() == WebssType::TEMPLATE_HEAD_STANDARD || getTypeRaw() == WebssType::TEMPLATE_HEAD_TEXT); return *theadStandard; }
const TemplateBinary& Webss::getTemplateBinaryRaw() const { assert(getTypeRaw() == WebssType::TEMPLATE_BINARY); return *templBinary; }
const TemplateScoped& Webss::getTemplateScopedRaw() const { assert(getTypeRaw() == WebssType::TEMPLATE_SCOPED); return *templScoped; }
const TemplateStandard& Webss::getTemplateStandardRaw() const { assert(getTypeRaw() == WebssType::TEMPLATE_STANDARD || getTypeRaw() == WebssType::TEMPLATE_TEXT); return *templStandard; }
const BlockHead& Webss::getBlockHeadRaw() const { assert(getTypeRaw() == WebssType::BLOCK_HEAD); return *bhead; }
const Block& Webss::getBlockRaw() const { assert(getTypeRaw() == WebssType::BLOCK); return *block; }

Entity& Webss::getEntityRaw() { assert(getTypeRaw() == WebssType::ENTITY); return ent; }
Default& Webss::getDefaultRaw() { assert(getTypeRaw() == WebssType::DEFAULT); return tDefault; }
Namespace& Webss::getNamespaceRaw() { assert(getTypeRaw() == WebssType::NAMESPACE); return nspace; }
Enum& Webss::getEnumRaw() { assert(getTypeRaw() == WebssType::ENUM); return tEnum; }

std::string& Webss::getStringRaw() { assert(getTypeRaw() == WebssType::PRIMITIVE_STRING); return *tString; }
Document& Webss::getDocumentRaw() { assert(getTypeRaw() == WebssType::DOCUMENT); return *document; }
Dictionary& Webss::getDictionaryRaw() { assert(getTypeRaw() == WebssType::DICTIONARY); return *dict; }
List& Webss::getListRaw() { assert(getTypeRaw() == WebssType::LIST || getTypeRaw() == WebssType::LIST_TEXT); return *list; }
Tuple& Webss::getTupleRaw() { assert(getTypeRaw() == WebssType::TUPLE || getTypeRaw() == WebssType::TUPLE_TEXT || getTypeRaw() == WebssType::TUPLE_ABSTRACT); return *tuple; }
TemplateHeadBinary& Webss::getTemplateHeadBinaryRaw() { assert(getTypeRaw() == WebssType::TEMPLATE_HEAD_BINARY); return *theadBinary; }
TemplateHeadScoped& Webss::getTemplateHeadScopedRaw() { assert(getTypeRaw() == WebssType::TEMPLATE_HEAD_SCOPED); return *theadScoped; }
TemplateHeadStandard& Webss::getTemplateHeadStandardRaw() { assert(getTypeRaw() == WebssType::TEMPLATE_HEAD_STANDARD || getTypeRaw() == WebssType::TEMPLATE_HEAD_TEXT); return *theadStandard; }
TemplateBinary& Webss::getTemplateBinaryRaw() { assert(getTypeRaw() == WebssType::TEMPLATE_BINARY); return *templBinary; }
TemplateScoped& Webss::getTemplateScopedRaw() { assert(getTypeRaw() == WebssType::TEMPLATE_SCOPED); return *templScoped; }
TemplateStandard& Webss::getTemplateStandardRaw() { assert(getTypeRaw() == WebssType::TEMPLATE_STANDARD || getTypeRaw() == WebssType::TEMPLATE_TEXT); return *templStandard; }
BlockHead& Webss::getBlockHeadRaw() { assert(getTypeRaw() == WebssType::BLOCK_HEAD); return *bhead; }
Block& Webss::getBlockRaw() { assert(getTypeRaw() == WebssType::BLOCK); return *block; }