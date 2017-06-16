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

Thead::Thead(Entity ent) : type(TypeThead::ENTITY), ent(move(ent)) { assert(ent.getContent().isThead()); }
Thead::Thead(TemplateHeadBinary theadBinary, TheadOptions options) : type(TypeThead::BINARY), options(move(options)), theadBinary(new TemplateHeadBinary(move(theadBinary))) {}
Thead::Thead(TemplateHeadStandard theadStandard, TheadOptions options) : type(TypeThead::STANDARD), options(move(options)), theadStandard(new TemplateHeadStandard(move(theadStandard))) {}
Thead::Thead(TemplateHeadSelf, TheadOptions options) : type(TypeThead::SELF), options(move(options)) {}

void Thead::destroyUnion()
{
	switch (type)
	{
	case TypeThead::NONE: case TypeThead::SELF:
		break;
	case TypeThead::ENTITY:
		ent.~Entity();
		break;
	case TypeThead::BINARY:
		delete theadBinary;
		break;
	case TypeThead::STANDARD:
		delete theadStandard;
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
	case TypeThead::BINARY:
		theadBinary = o.theadBinary;
		break;
	case TypeThead::STANDARD:
		theadStandard = o.theadStandard;
		break;
	default:
		assert(false);
	}
	type = o.type;
	o.type = TypeThead::NONE;
	options = o.options;
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
	case TypeThead::BINARY:
		theadBinary = new TemplateHeadBinary(*o.theadBinary);
		break;
	case TypeThead::STANDARD:
		theadStandard = new TemplateHeadStandard(*o.theadStandard);
		break;
	default:
		assert(false);
	}
	type = o.type;
	options = o.options;
}


bool Thead::operator==(const Thead& o) const
{
	if (this == &o)
		return true;
	if (type != o.type || options.isText != o.options.isText || options.isPlus != o.options.isPlus)
		return false;
	switch (o.type)
	{
	case TypeThead::NONE: case TypeThead::SELF:
		return true;
	case TypeThead::ENTITY:
		return ent == o.ent;
	case TypeThead::BINARY:
		return *theadBinary == *o.theadBinary;
	case TypeThead::STANDARD:
		return *theadStandard == *o.theadStandard;
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

const TemplateHeadBinary& Thead::getTheadBinary() const { PATTERN_GET_CONST_SAFE(TypeThead::BINARY, getTheadBinaryRaw, "expected template head binary"); }
const TemplateHeadStandard& Thead::getTheadStandard() const { PATTERN_GET_CONST_SAFE(TypeThead::STANDARD, getTheadStandardRaw, "expected template head standard"); }

bool Thead::isNone() const { return getType() == TypeThead::NONE; }
bool Thead::isTheadBinary() const { return getType() == TypeThead::BINARY; }
bool Thead::isTheadStandard() const { return getType() == TypeThead::STANDARD; }

TypeThead Thead::getTypeRaw() const { return type; }

const Entity& Thead::getEntityRaw() const { assert(getTypeRaw() == TypeThead::ENTITY); return ent; }
const TemplateHeadBinary& Thead::getTheadBinaryRaw() const { assert(getTypeRaw() == TypeThead::BINARY); return *theadBinary; }
const TemplateHeadStandard& Thead::getTheadStandardRaw() const { assert(getTypeRaw() == TypeThead::STANDARD); return *theadStandard; }

Entity& Thead::getEntityRaw() { assert(getTypeRaw() == TypeThead::ENTITY); return ent; }
TemplateHeadBinary& Thead::getTheadBinaryRaw() { assert(getTypeRaw() == TypeThead::BINARY); return *theadBinary; }
TemplateHeadStandard& Thead::getTheadStandardRaw() { assert(getTypeRaw() == TypeThead::STANDARD); return *theadStandard; }

TheadOptions Thead::getOptions() const { return options; }

bool Thead::isText() const { return options.isText; }
bool Thead::isPlus() const { return options.isPlus; }