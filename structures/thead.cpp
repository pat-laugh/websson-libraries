//MIT License
//Copyright 2017 Patrick Laughrea
#include "thead.hpp"

#include <cassert>

#include "paramStandard.hpp"

using namespace std;
using namespace webss;

Thead::Thead() {}
Thead::~Thead() { destroyUnion(); }

Thead::Thead(Thead&& o) { copyUnion(move(o)); }
Thead::Thead(const Thead& o) { copyUnion(o); }

Thead& Thead::operator=(Thead o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}

Thead::Thead(Entity ent) : type(TypeThead::ENTITY), options(ent.getContent().getThead().options), ent(ent) { assert(ent.getContent().isThead()); }
Thead::Thead(Entity ent, TheadOptions options) : type(TypeThead::ENTITY), options(move(options)), ent(move(ent)) { assert(ent.getContent().isThead()); }
Thead::Thead(TheadSelf, TheadOptions options) : type(TypeThead::SELF), options(move(options)) {}
Thead::Thead(TheadBin theadBin, TheadOptions options) : type(TypeThead::BIN), options(move(options)), theadBin(new TheadBin(move(theadBin))) {}
Thead::Thead(TheadStd theadStd, TheadOptions options) : type(TypeThead::STD), options(move(options)), theadStd(new TheadStd(move(theadStd))) {}
Thead::Thead(TheadBin theadBin, Entity base, TheadOptions options)
	: type(TypeThead::BIN), options(move(options)), theadBin(new TheadBin(move(theadBin))), base(move(base))
	{ assert(base.getContent().isThead() && base.getContent().getThead().isTheadBin()); }
Thead::Thead(TheadStd theadStd, Entity base, TheadOptions options)
	: type(TypeThead::STD), options(move(options)), theadStd(new TheadStd(move(theadStd))), base(move(base))
	{ assert(base.getContent().isThead() && base.getContent().getThead().isTheadStd()); }

void Thead::destroyUnion()
{
	switch (type)
	{
	case TypeThead::NONE: case TypeThead::SELF:
		break;
	case TypeThead::ENTITY:
		ent.~Entity();
		break;
	case TypeThead::BIN:
		delete theadBin;
		break;
	case TypeThead::STD:
		delete theadStd;
		break;
	default:
		assert(false);
	}
	type = TypeThead::NONE;
}

void Thead::copyUnion(Thead&& o)
{
	switch (o.type)
	{
	case TypeThead::NONE: case TypeThead::SELF:
		break;
	case TypeThead::ENTITY:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case TypeThead::BIN:
		theadBin = o.theadBin;
		break;
	case TypeThead::STD:
		theadStd = o.theadStd;
		break;
	default:
		assert(false);
	}
	type = o.type;
	o.type = TypeThead::NONE;
	options = o.options;
	base = move(o.base);
}

void Thead::copyUnion(const Thead& o)
{
	switch (o.type)
	{
	case TypeThead::NONE: case TypeThead::SELF:
		break;
	case TypeThead::ENTITY:
		new (&ent) Entity(o.ent);
		break;
	case TypeThead::BIN:
		theadBin = new TheadBin(*o.theadBin);
		break;
	case TypeThead::STD:
		theadStd = new TheadStd(*o.theadStd);
		break;
	default:
		assert(false);
	}
	type = o.type;
	options = o.options;
	base = o.base;
}


bool Thead::operator==(const Thead& o) const
{
	if (this == &o)
		return true;
	if (type != o.type || options.isText != o.options.isText || options.isPlus != o.options.isPlus || base != o.base)
		return false;
	switch (o.type)
	{
	case TypeThead::NONE: case TypeThead::SELF:
		return true;
	case TypeThead::ENTITY:
		return ent == o.ent;
	case TypeThead::BIN:
		return *theadBin == *o.theadBin;
	case TypeThead::STD:
		return *theadStd == *o.theadStd;
	default:
		assert(false); throw domain_error("");
	}
}
bool Thead::operator!=(const Thead& o) const { return !(*this == o); }

const Thead& Thead::getTheadLast() const
{
	if (type == TypeThead::ENTITY)
		return ent.getContent().getThead().getTheadLast();
	return *this;
}

TypeThead Thead::getType() const
{
	return getTheadLast().getTypeRaw();
}

#define PATTERN_GET_CONST_SAFE(Type, Func, ErrorMessage) \
const auto& thead = getTheadLast(); \
if (thead.getTypeRaw() == Type) \
	return thead.Func(); \
else \
	throw runtime_error(ErrorMessage);

const TheadBin& Thead::getTheadBin() const { PATTERN_GET_CONST_SAFE(TypeThead::BIN, getTheadBinRaw, "expected template head binary"); }
const TheadStd& Thead::getTheadStd() const { PATTERN_GET_CONST_SAFE(TypeThead::STD, getTheadStdRaw, "expected template head standard"); }

bool Thead::isNone() const { return getType() == TypeThead::NONE; }
bool Thead::isTheadBin() const { return getType() == TypeThead::BIN; }
bool Thead::isTheadStd() const { return getType() == TypeThead::STD; }

TypeThead Thead::getTypeRaw() const { return type; }

bool Thead::hasEntity() const { return getTypeRaw() == TypeThead::ENTITY; }

const Entity& Thead::getEntityRaw() const { assert(getTypeRaw() == TypeThead::ENTITY); return ent; }
const TheadBin& Thead::getTheadBinRaw() const { assert(getTypeRaw() == TypeThead::BIN); return *theadBin; }
const TheadStd& Thead::getTheadStdRaw() const { assert(getTypeRaw() == TypeThead::STD); return *theadStd; }

Entity& Thead::getEntityRaw() { assert(getTypeRaw() == TypeThead::ENTITY); return ent; }
TheadBin& Thead::getTheadBinRaw() { assert(getTypeRaw() == TypeThead::BIN); return *theadBin; }
TheadStd& Thead::getTheadStdRaw() { assert(getTypeRaw() == TypeThead::STD); return *theadStd; }

TheadOptions Thead::getOptions() const { return options; }

bool Thead::isText() const { return options.isText; }
bool Thead::isPlus() const { return options.isPlus; }

bool Thead::hasBase() const { return base.hasBody(); }
const Entity& Thead::getBase() const { return base; }