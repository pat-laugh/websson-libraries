//MIT License
//Copyright 2017 Patrick Laughrea
#include "thead.hpp"

#include <cassert>

#include "paramStandard.hpp"
#include "theadFun.hpp"
#include "tuple.hpp"

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

Thead::Thead(Entity ent) : type(TypeThead::ENTITY), options(ent.getContent().getThead().options), ent(ent) { assert(this->ent.getContent().isThead()); }
Thead::Thead(Entity ent, TheadOptions options) : type(TypeThead::ENTITY), options(move(options)), ent(move(ent)) { assert(this->ent.getContent().isThead()); }
Thead::Thead(TheadSelf) : type(TypeThead::SELF) {}
Thead::Thead(TheadBin theadBin, TheadOptions options) : type(TypeThead::BIN), options(move(options)), theadBin(new TheadBin(move(theadBin))) {}
Thead::Thead(TheadFun theadFun, TheadOptions options) : type(TypeThead::FUN), options(move(options)), theadFun(new TheadFun(move(theadFun))) {}
Thead::Thead(TheadStd theadStd, TheadOptions options) : type(TypeThead::STD), options(move(options)), theadStd(new TheadStd(move(theadStd))) {}
Thead::Thead(TheadBin theadBin, Entity base, TheadOptions options)
	: type(TypeThead::BIN), options(move(options)), theadBin(new TheadBin(move(theadBin))), base(move(base))
	{ assert(this->base.getContent().isThead() && this->base.getContent().getThead().isTheadBin()); }
Thead::Thead(TheadFun theadFun, Entity base, TheadOptions options)
	: type(TypeThead::FUN), options(move(options)), theadFun(new TheadFun(move(theadFun))), base(move(base))
	{ assert(this->base.getContent().isThead() && this->base.getContent().getThead().isTheadFun()); }
Thead::Thead(TheadStd theadStd, Entity base, TheadOptions options)
	: type(TypeThead::STD), options(move(options)), theadStd(new TheadStd(move(theadStd))), base(move(base))
	{ assert(this->base.getContent().isThead() && this->base.getContent().getThead().isTheadStd()); }
Thead::Thead(TheadFun theadFun, Entity base, TheadOptions options, Tuple modifierTuple)
	: type(TypeThead::FUN), options(move(options)), theadFun(new TheadFun(move(theadFun))), base(move(base)), modifierTuple(new Tuple(move(modifierTuple)))
{
	assert(this->base.getContent().isThead() && this->base.getContent().getThead().isTheadStd());
}
Thead::Thead(TheadStd theadStd, Entity base, TheadOptions options, Tuple modifierTuple)
: type(TypeThead::STD), options(move(options)), theadStd(new TheadStd(move(theadStd))), base(move(base)), modifierTuple(new Tuple(move(modifierTuple)))
{
assert(this->base.getContent().isThead() && this->base.getContent().getThead().isTheadStd());
}

void Thead::destroyUnion()
{
	switch (type)
	{
	default: assert(false);
	case TypeThead::NONE: case TypeThead::SELF:
		break;
	case TypeThead::ENTITY:
		ent.~Entity();
		break;
	case TypeThead::BIN:
		delete theadBin;
		break;
	case TypeThead::FUN:
		delete theadFun;
		break;
	case TypeThead::STD:
		delete theadStd;
		break;
	}
	type = TypeThead::NONE;
	if (modifierTuple != nullptr)
		delete modifierTuple;
}

void Thead::copyUnion(Thead&& o)
{
	switch (o.type)
	{
	default: assert(false);
	case TypeThead::NONE: case TypeThead::SELF:
		break;
	case TypeThead::ENTITY:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case TypeThead::BIN:
		theadBin = o.theadBin;
		break;
	case TypeThead::FUN:
		theadFun = o.theadFun;
		break;
	case TypeThead::STD:
		theadStd = o.theadStd;
		break;
	}
	type = o.type;
	o.type = TypeThead::NONE;
	options = o.options;
	base = move(o.base);
	modifierTuple = o.modifierTuple;
	o.modifierTuple = nullptr;
}

void Thead::copyUnion(const Thead& o)
{
	switch (o.type)
	{
	default: assert(false);
	case TypeThead::NONE: case TypeThead::SELF:
		break;
	case TypeThead::ENTITY:
		new (&ent) Entity(o.ent);
		break;
	case TypeThead::BIN:
		theadBin = new TheadBin(*o.theadBin);
		break;
	case TypeThead::FUN:
		theadFun = new TheadFun(*o.theadFun);
		break;
	case TypeThead::STD:
		theadStd = new TheadStd(*o.theadStd);
		break;
	}
	type = o.type;
	options = o.options;
	base = o.base;
	if (o.modifierTuple != nullptr)
		modifierTuple = new Tuple(*o.modifierTuple);
}


bool Thead::operator==(const Thead& o) const
{
	if (this == &o)
		return true;
	if (getType() != o.getType() || options.isText != o.options.isText || options.isPlus != o.options.isPlus || !equalPtrs(modifierTuple, o.modifierTuple))
		return false;
	switch (getType())
	{
	default: assert(false);
	case TypeThead::NONE: case TypeThead::SELF:
		return true;
	case TypeThead::BIN:
		return getTheadBin() == o.getTheadBin();
	case TypeThead::FUN:
		return getTheadFun() == o.getTheadFun();
	case TypeThead::STD:
		return getTheadStd() == o.getTheadStd();
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

#define PatternGetConstSafe(Type, Func, ErrorMessage) do { \
const auto& thead = getTheadLast(); \
if (thead.getTypeRaw() == Type) \
	return thead.Func(); \
throw runtime_error(ErrorMessage); } while (false)

const TheadBin& Thead::getTheadBin() const { PatternGetConstSafe(TypeThead::BIN, getTheadBinRaw, "expected template head binary"); }
const TheadFun& Thead::getTheadFun() const { PatternGetConstSafe(TypeThead::FUN, getTheadFunRaw, "expected template head function"); }
const TheadStd& Thead::getTheadStd() const { PatternGetConstSafe(TypeThead::STD, getTheadStdRaw, "expected template head standard"); }

bool Thead::isNone() const { return getType() == TypeThead::NONE; }
bool Thead::isTheadBin() const { return getType() == TypeThead::BIN; }
bool Thead::isTheadFun() const { return getType() == TypeThead::FUN; }
bool Thead::isTheadStd() const { return getType() == TypeThead::STD; }

TypeThead Thead::getTypeRaw() const { return type; }

bool Thead::hasEntity() const { return getTypeRaw() == TypeThead::ENTITY; }

const Entity& Thead::getEntityRaw() const { assert(getTypeRaw() == TypeThead::ENTITY); return ent; }
const TheadBin& Thead::getTheadBinRaw() const { assert(getTypeRaw() == TypeThead::BIN); return *theadBin; }
const TheadFun& Thead::getTheadFunRaw() const { assert(getTypeRaw() == TypeThead::FUN); return *theadFun; }
const TheadStd& Thead::getTheadStdRaw() const { assert(getTypeRaw() == TypeThead::STD); return *theadStd; }

Entity& Thead::getEntityRaw() { assert(getTypeRaw() == TypeThead::ENTITY); return ent; }
TheadBin& Thead::getTheadBinRaw() { assert(getTypeRaw() == TypeThead::BIN); return *theadBin; }
TheadFun& Thead::getTheadFunRaw() { assert(getTypeRaw() == TypeThead::FUN); return *theadFun; }
TheadStd& Thead::getTheadStdRaw() { assert(getTypeRaw() == TypeThead::STD); return *theadStd; }

TheadOptions Thead::getOptions() const { return options; }

bool Thead::isText() const { return options.isText; }
bool Thead::isPlus() const { return options.isPlus; }

bool Thead::hasBase() const { return base.hasBody(); }
const Entity& Thead::getBase() const { return base; }

bool Thead::hasModifierTuple() const { return modifierTuple != nullptr; }
const Tuple& Thead::getModifierTuple() const { assert(hasModifierTuple()); return *modifierTuple; }