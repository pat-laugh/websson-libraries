//MIT License
//Copyright 2017 Patrick Laughrea
#include "paramBinary.hpp"

#include <cassert>

#include "utils.hpp"

using namespace std;
using namespace webss;

#define EntNumInt entNumber.getContent().getInt()

//ParamBin::SizeHead

using SizeHead = ParamBin::SizeHead;

SizeHead::SizeHead() {}
SizeHead::SizeHead(Keyword keyword) : type(Type::KEYWORD)
{
	switch (keyword)
	{
	default: assert(false);
	case Keyword::BOOL: case Keyword::INT8: case Keyword::INT16: case Keyword::INT32: case Keyword::INT64:
	case Keyword::FLOAT: case Keyword::DOUBLE: case Keyword::VARINT:
		this->keyword = keyword;
		break;
	case Keyword::STRING:
		type = Type::EMPTY;
		break;
	}
}

SizeHead::SizeHead(WebssBinSize num) : type(num == 0 ? Type::EMPTY : Type::NUMBER), number(num) {}
SizeHead::SizeHead(TheadSelf) : type(Type::SELF) {}
SizeHead::SizeHead(Type type) : type(type)
{
	assert(type == Type::NONE || type == Type::EMPTY || type == Type::SELF);
}

SizeHead::~SizeHead() { destroyUnion(); }

SizeHead::SizeHead(TheadBin&& o) : type(Type::THEAD), thead(new TheadBin(move(o))) {}
SizeHead::SizeHead(const TheadBin& o) : type(Type::THEAD), thead(new TheadBin(o)) {}

SizeHead::SizeHead(SizeHead&& o) { copyUnion(move(o)); }
SizeHead::SizeHead(const SizeHead& o) { copyUnion(o); }

SizeHead& SizeHead::operator=(SizeHead&& o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}
SizeHead& SizeHead::operator=(const SizeHead& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(o);
	}
	return *this;
}

bool SizeHead::operator==(const SizeHead& o) const
{
	if (this == &o)
		return true;
	if (type != o.type || !equalPtrs(defaultValue, o.defaultValue))
		return false;
	switch (o.type)
	{
	default: assert(false);
	case Type::NONE: case Type::EMPTY: case Type::SELF:
		return true;
	case Type::KEYWORD:
		return keyword == o.keyword;
	case Type::NUMBER: case Type::BITS:
		return number == o.number;
	case Type::THEAD:
		return *thead == *o.thead;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_THEAD: case Type::ENTITY_BITS:
		return ent == o.ent;
	}
}
bool SizeHead::operator!=(const SizeHead& o) const { return !(*this == o); }

bool SizeHead::isEmpty() const { return type == Type::EMPTY || type == Type::EMPTY_ENTITY_NUMBER; }
bool SizeHead::isKeyword() const { return type == Type::KEYWORD; }
bool SizeHead::isBool() const { return isKeyword() && keyword == Keyword::BOOL; }
bool SizeHead::isTheadBin() const { return type == Type::ENTITY_THEAD || type == Type::THEAD; }
bool SizeHead::hasEntity() const { return type == Type::ENTITY_THEAD || type == Type::ENTITY_NUMBER || type == Type::EMPTY_ENTITY_NUMBER || type == Type::ENTITY_BITS; }

bool SizeHead::hasDefaultValue() const { return defaultValue.get() != nullptr; }
bool SizeHead::isTheadSelf() const { return type == Type::SELF; }

SizeHead::Type SizeHead::getType() const { return type; }
Keyword SizeHead::getKeyword() const
{
	assert(isKeyword());
	return keyword;
}

const Webss& SizeHead::getDefaultValue() const
{
	assert(hasDefaultValue());
	return *defaultValue;
}
const shared_ptr<Webss>& SizeHead::getDefaultPointer() const
{
	assert(hasDefaultValue());
	return defaultValue;
}
void SizeHead::setDefaultValue(Webss&& value) { defaultValue = shared_ptr<Webss>(new Webss(move(value))); }

const Entity& SizeHead::getEntity() const
{
	assert(hasEntity());
	return ent;
}

const TheadBin& SizeHead::getThead() const
{
	assert(isTheadBin());
	return type == Type::ENTITY_THEAD ? ent.getContent().getThead().getTheadBin() : *thead;
}

WebssBinSize SizeHead::size() const
{
	switch (type)
	{
	default: assert(false);
	case Type::ENTITY_NUMBER: case Type::ENTITY_BITS:
		return static_cast<WebssBinSize>(ent.getContent().getInt());
	case Type::NUMBER: case Type::BITS:
		return number;
	}
}

SizeHead::SizeHead(const Entity& entThead) : type(Type::ENTITY_THEAD), ent(entThead)
{
	assert(entThead.getContent().isThead() && entThead.getContent().getThead().isTheadBin());
}
SizeHead::SizeHead(const Entity& entNumber, bool)
	: type(EntNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
{
	assert(EntNumInt >= 0 && static_cast<WebssBinSize>(EntNumInt) <= std::numeric_limits<WebssBinSize>::max());
}
SizeHead::SizeHead(WebssBinSize num, bool) : type(Type::BITS), number(num)
{
	assert(num > 0 && num <= 8);
}
SizeHead::SizeHead(const Entity& entNumber, bool, bool) : type(Type::ENTITY_BITS), ent(entNumber)
{
	assert(EntNumInt > 0 && EntNumInt <= 8);
}

void SizeHead::destroyUnion()
{
	switch (type)
	{
	case Type::THEAD:
		delete thead;
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_THEAD: case Type::ENTITY_BITS:
		ent.~Entity();
		break;
	default:
		break;
	}
	type = Type::NONE;
}

void SizeHead::copyUnion(SizeHead&& o)
{
	switch (o.type)
	{
	default: assert(false);
	case Type::NONE: case Type::EMPTY: case Type::SELF:
		break;
	case Type::KEYWORD:
		keyword = o.keyword;
		break;
	case Type::NUMBER: case Type::BITS:
		number = o.number;
		break;
	case Type::THEAD:
		thead = o.thead;
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_THEAD: case Type::ENTITY_BITS:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	}
	type = o.type;
	o.type = Type::NONE;

	if (o.hasDefaultValue())
		defaultValue = move(o.defaultValue);
}
void SizeHead::copyUnion(const SizeHead& o)
{
	switch (o.type)
	{
	default: assert(false);
	case Type::NONE: case Type::EMPTY: case Type::SELF:
		break;
	case Type::KEYWORD:
		keyword = o.keyword;
		break;
	case Type::NUMBER: case Type::BITS:
		number = o.number;
		break;
	case Type::THEAD:
		thead = new TheadBin(*o.thead);
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_THEAD: case Type::ENTITY_BITS:
		new (&ent) Entity(o.ent);
		break;
	}
	type = o.type;

	if (o.hasDefaultValue())
		defaultValue = o.defaultValue;
}

//ParamBin::SizeArray

using SizeArray = ParamBin::SizeArray;

SizeArray::SizeArray() {}
SizeArray::SizeArray(Type type) : type(type)
{
	assert(type == Type::NONE || type == Type::EMPTY || type == Type::ONE);
}
SizeArray::SizeArray(const Entity& entNumber)
	: type(EntNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
{
	assert(EntNumInt >= 0 && static_cast<WebssBinSize>(EntNumInt) <= numeric_limits<WebssBinSize>::max());
}
SizeArray::SizeArray(WebssBinSize num) : type(num == 0 ? Type::EMPTY : Type::NUMBER), number(num) {}

SizeArray::~SizeArray() { destroyUnion(); }

SizeArray::SizeArray(SizeArray&& o) { copyUnion(move(o)); }
SizeArray::SizeArray(const SizeArray& o) { copyUnion(o); }

SizeArray& SizeArray::operator=(SizeArray&& o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}
SizeArray& SizeArray::operator=(const SizeArray& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(o);
	}
	return *this;
}

bool SizeArray::operator==(const SizeArray& o) const
{
	if (this == &o)
		return true;
	if (type != o.type)
		return false;
	switch (o.type)
	{
	default: assert(false);
	case Type::NONE: case Type::EMPTY: case Type::ONE:
		return true;
	case Type::NUMBER:
		return number == o.number;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		return ent == o.ent;
	}
}
bool SizeArray::operator!=(const SizeArray& o) const { return !(*this == o); }

bool SizeArray::isEmpty() const { return type == Type::EMPTY || type == Type::EMPTY_ENTITY_NUMBER; }
bool SizeArray::isOne() const { return type == Type::ONE; }
bool SizeArray::hasEntity() const { return type == Type::ENTITY_NUMBER || type == Type::EMPTY_ENTITY_NUMBER; }

SizeArray::Type SizeArray::getType() const { return type; }

WebssBinSize SizeArray::size() const
{
	assert(type == Type::NUMBER || type == Type::ENTITY_NUMBER);
	if (type == Type::NUMBER)
		return number;
	else
		return static_cast<WebssBinSize>(ent.getContent().getInt());
}

const Entity& SizeArray::getEntity() const
{
	assert(hasEntity());
	return ent;
}

void SizeArray::destroyUnion()
{
	if (hasEntity())
		ent.~Entity();
	type = Type::NONE;
}

void SizeArray::copyUnion(SizeArray&& o)
{
	switch (o.type)
	{
	default: assert(false);
	case Type::NONE: case Type::EMPTY: case Type::ONE:
		break;
	case Type::NUMBER:
		number = o.number;
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	}
	type = o.type;
	o.type = Type::NONE;
}
void SizeArray::copyUnion(const SizeArray& o)
{
	switch (o.type)
	{
	default: assert(false);
	case Type::NONE: case Type::EMPTY: case Type::ONE:
		break;
	case Type::NUMBER:
		number = o.number;
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		new (&ent) Entity(o.ent);
		break;
	}
	type = o.type;
}

//ParamBin

ParamBin::ParamBin() {}
ParamBin::ParamBin(SizeHead&& sizeHead, SizeArray&& sizeArray) : sizeHead(move(sizeHead)), sizeArray(move(sizeArray)) {}
ParamBin::ParamBin(const SizeHead& sizeHead, const SizeArray& sizeArray) : sizeHead(sizeHead), sizeArray(sizeArray) {}
ParamBin::~ParamBin() {}

ParamBin::ParamBin(ParamBin&& o) : sizeHead(move(o.sizeHead)), sizeArray(move(o.sizeArray)) {}
ParamBin::ParamBin(const ParamBin& o) : sizeHead(o.sizeHead), sizeArray(o.sizeArray) {}

ParamBin& ParamBin::operator=(ParamBin&& o)
{
	sizeHead = move(o.sizeHead);
	sizeArray = move(o.sizeArray);
	return *this;
}
ParamBin& ParamBin::operator=(const ParamBin& o)
{
	if (this != &o)
	{
		sizeHead = o.sizeHead;
		sizeArray = o.sizeArray;
	}
	return *this;
}

bool ParamBin::operator==(const ParamBin& o) const { return (this == &o) || (sizeHead == o.sizeHead && sizeArray == o.sizeArray); }
bool ParamBin::operator!=(const ParamBin& o) const { return !(*this == o); }

const SizeHead& ParamBin::getSizeHead() const { return sizeHead; }
const SizeArray& ParamBin::getSizeArray() const { return sizeArray; }
bool ParamBin::hasDefaultValue() const { return getSizeHead().hasDefaultValue(); }
const shared_ptr<Webss>& ParamBin::getDefaultPointer() const { return getSizeHead().getDefaultPointer(); }
bool ParamBin::isTheadBin() const { return getSizeHead().isTheadBin(); }
bool ParamBin::isTheadSelf() const { return getSizeHead().isTheadSelf(); }
const TheadBin& ParamBin::getThead() const { return getSizeHead().getThead(); }
