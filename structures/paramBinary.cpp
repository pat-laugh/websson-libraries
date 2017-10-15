//MIT License
//Copyright 2017 Patrick Laughrea
#include "paramBinary.hpp"

#include <cassert>

#include "utils.hpp"

using namespace std;
using namespace webss;

#define entNumInt entNumber.getContent().getInt()

//ParamBin::SizeHead

using SizeHead = ParamBin::SizeHead;

SizeHead::SizeHead() {}
SizeHead::SizeHead(Keyword keyword) : type(Type::KEYWORD)
{
	switch (keyword)
	{
	case Keyword::BOOL: case Keyword::INT8: case Keyword::INT16: case Keyword::INT32: case Keyword::INT64:
	case Keyword::FLOAT: case Keyword::DOUBLE: case Keyword::VARINT:
		this->keyword = keyword;
		break;
	case Keyword::STRING:
		type = Type::EMPTY;
		break;
	default:
		assert(false); throw domain_error("invalid binary type: " + keyword.toString());
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
	default:
		assert(false); throw domain_error("");
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
	case Type::ENTITY_NUMBER: case Type::ENTITY_BITS:
		return static_cast<WebssBinSize>(ent.getContent().getInt());
	case Type::NUMBER: case Type::BITS:
		return number;
	default:
		assert(false); throw domain_error("");
	}
}

SizeHead::SizeHead(const Entity& entThead) : type(Type::ENTITY_THEAD), ent(entThead)
{
	assert(entThead.getContent().isThead() && entThead.getContent().getThead().isTheadBin());
}
SizeHead::SizeHead(const Entity& entNumber, bool)
	: type(entNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
{
	assert(entNumInt >= 0 && static_cast<WebssBinSize>(entNumInt) <= std::numeric_limits<WebssBinSize>::max());
}
SizeHead::SizeHead(WebssBinSize num, bool) : type(Type::BITS), number(num)
{
	assert(num > 0 && num <= 8);
}
SizeHead::SizeHead(const Entity& entNumber, bool, bool) : type(Type::ENTITY_BITS), ent(entNumber)
{
	assert(entNumInt > 0 && entNumInt <= 8);
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
	default:
		assert(false);
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
	default:
		assert(false);
	}
	type = o.type;

	if (o.hasDefaultValue())
		defaultValue = o.defaultValue;
}

//ParamBin::SizeList

using SizeList = ParamBin::SizeList;

SizeList::SizeList() {}
SizeList::SizeList(Type type) : type(type)
{
	assert(type == Type::NONE || type == Type::EMPTY || type == Type::ONE);
}
SizeList::SizeList(const Entity& entNumber)
	: type(entNumInt == 0 ? Type::EMPTY_ENTITY_NUMBER : Type::ENTITY_NUMBER), ent(entNumber)
{
	assert(entNumInt >= 0 && static_cast<WebssBinSize>(entNumInt) <= numeric_limits<WebssBinSize>::max());
}
SizeList::SizeList(WebssBinSize num) : type(num == 0 ? Type::EMPTY : Type::NUMBER), number(num) {}

SizeList::~SizeList() { destroyUnion(); }

SizeList::SizeList(SizeList&& o) { copyUnion(move(o)); }
SizeList::SizeList(const SizeList& o) { copyUnion(o); }

SizeList& SizeList::operator=(SizeList&& o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}
SizeList& SizeList::operator=(const SizeList& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(o);
	}
	return *this;
}

bool SizeList::operator==(const SizeList& o) const
{
	if (this == &o)
		return true;
	if (type != o.type)
		return false;
	switch (o.type)
	{
	case Type::NONE: case Type::EMPTY: case Type::ONE:
		return true;
	case Type::NUMBER:
		return number == o.number;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		return ent == o.ent;
	default:
		assert(false); throw domain_error("");
	}
}
bool SizeList::operator!=(const SizeList& o) const { return !(*this == o); }

bool SizeList::isEmpty() const { return type == Type::EMPTY || type == Type::EMPTY_ENTITY_NUMBER; }
bool SizeList::isOne() const { return type == Type::ONE; }
bool SizeList::hasEntity() const { return type == Type::ENTITY_NUMBER || type == Type::EMPTY_ENTITY_NUMBER; }

SizeList::Type SizeList::getType() const { return type; }

WebssBinSize SizeList::size() const
{
	assert(type == Type::NUMBER || type == Type::ENTITY_NUMBER);
	if (type == Type::NUMBER)
		return number;
	else
		return static_cast<WebssBinSize>(ent.getContent().getInt());
}

const Entity& SizeList::getEntity() const
{
	assert(hasEntity());
	return ent;
}

void SizeList::destroyUnion()
{
	if (hasEntity())
		ent.~Entity();
	type = Type::NONE;
}

void SizeList::copyUnion(SizeList&& o)
{
	switch (o.type)
	{
	case Type::NONE: case Type::EMPTY: case Type::ONE:
		break;
	case Type::NUMBER:
		number = o.number;
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	default:
		assert(false);
	}
	type = o.type;
	o.type = Type::NONE;
}
void SizeList::copyUnion(const SizeList& o)
{
	switch (o.type)
	{
	case Type::NONE: case Type::EMPTY: case Type::ONE:
		break;
	case Type::NUMBER:
		number = o.number;
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		new (&ent) Entity(o.ent);
		break;
	default:
		assert(false);
	}
	type = o.type;
}

//ParamBin

ParamBin::ParamBin() {}
ParamBin::ParamBin(SizeHead&& sizeHead, SizeList&& sizeList) : sizeHead(move(sizeHead)), sizeList(move(sizeList)) {}
ParamBin::ParamBin(const SizeHead& sizeHead, const SizeList& sizeList) : sizeHead(sizeHead), sizeList(sizeList) {}
ParamBin::~ParamBin() {}

ParamBin::ParamBin(ParamBin&& o) : sizeHead(move(o.sizeHead)), sizeList(move(o.sizeList)) {}
ParamBin::ParamBin(const ParamBin& o) : sizeHead(o.sizeHead), sizeList(o.sizeList) {}

ParamBin& ParamBin::operator=(ParamBin&& o)
{
	sizeHead = move(o.sizeHead);
	sizeList = move(o.sizeList);
	return *this;
}
ParamBin& ParamBin::operator=(const ParamBin& o)
{
	if (this != &o)
	{
		sizeHead = o.sizeHead;
		sizeList = o.sizeList;
	}
	return *this;
}

bool ParamBin::operator==(const ParamBin& o) const { return (this == &o) || (sizeHead == o.sizeHead && sizeList == o.sizeList); }
bool ParamBin::operator!=(const ParamBin& o) const { return !(*this == o); }

const SizeHead& ParamBin::getSizeHead() const { return sizeHead; }
const SizeList& ParamBin::getSizeList() const { return sizeList; }
bool ParamBin::hasDefaultValue() const { return getSizeHead().hasDefaultValue(); }
const shared_ptr<Webss>& ParamBin::getDefaultPointer() const { return getSizeHead().getDefaultPointer(); }
bool ParamBin::isTheadBin() const { return getSizeHead().isTheadBin(); }
bool ParamBin::isTheadSelf() const { return getSizeHead().isTheadSelf(); }
const TheadBin& ParamBin::getThead() const { return getSizeHead().getThead(); }
