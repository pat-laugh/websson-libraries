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

Thead::Thead(Entity ent) : type(TheadType::ENTITY), ent(move(ent)) { assert(ent.getContent().isThead()); }
Thead::Thead(TemplateHeadBinary theadBinary, TheadOptions options) : type(TheadType::BINARY), options(move(options)), theadBinary(new TemplateHeadBinary(move(theadBinary))) {}
Thead::Thead(TemplateHeadStandard theadStandard, TheadOptions options) : type(TheadType::STANDARD), options(move(options)), theadStandard(new TemplateHeadStandard(move(theadStandard))) {}
Thead::Thead(TemplateHeadSelf, TheadOptions options) : type(TheadType::SELF), options(move(options)) {}

void Thead::destroyUnion()
{
	switch (type)
	{
	case TheadType::NONE: case TheadType::SELF:
		break;
	case TheadType::ENTITY:
		ent.~Entity();
		break;
	case TheadType::BINARY:
		delete theadBinary;
		break;
	case TheadType::STANDARD:
		delete theadStandard;
		break;
	default:
		assert(false);
	}
	type = TheadType::NONE;
}

void Thead::copyUnion(Thead&& o)
{
	switch (o.type)
	{
	case TheadType::NONE: case TheadType::SELF:
		break;
	case TheadType::ENTITY:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case TheadType::BINARY:
		theadBinary = o.theadBinary;
		break;
	case TheadType::STANDARD:
		theadStandard = o.theadStandard;
		break;
	default:
		assert(false);
	}
	type = o.type;
	o.type = TheadType::NONE;
	options = o.options;
}

void Thead::copyUnion(const Thead& o)
{
	switch (o.type)
	{
	case TheadType::NONE: case TheadType::SELF:
		break;
	case TheadType::ENTITY:
		new (&ent) Entity(o.ent);
		break;
	case TheadType::BINARY:
		theadBinary = new TemplateHeadBinary(*o.theadBinary);
		break;
	case TheadType::STANDARD:
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
	case TheadType::NONE: case TheadType::SELF:
		return true;
	case TheadType::ENTITY:
		return ent == o.ent;
	case TheadType::BINARY:
		return *theadBinary == *o.theadBinary;
	case TheadType::STANDARD:
		return *theadStandard == *o.theadStandard;
	default:
		assert(false); throw domain_error("");
	}
}
bool Thead::operator!=(const Thead& o) const { return !(*this == o); }

const Thead& Thead::getTheadLast() const
{
	if (type == TheadType::ENTITY)
		return ent.getContent().getThead().getTheadLast();
	return *this;
}

TheadType Thead::getType() const
{
	return getTheadLast().getTypeRaw();
}

#define PATTERN_GET_CONST_SAFE(Type, Func, ErrorMessage) \
const auto& thead = getTheadLast(); \
if (thead.getTypeRaw() == Type) \
	return thead.Func(); \
else \
	throw runtime_error(ErrorMessage);

const TemplateHeadBinary& Thead::getTheadBinary() const { PATTERN_GET_CONST_SAFE(TheadType::BINARY, getTheadBinaryRaw, "expected template head binary"); }
const TemplateHeadStandard& Thead::getTheadStandard() const { PATTERN_GET_CONST_SAFE(TheadType::STANDARD, getTheadStandardRaw, "expected template head standard"); }

bool Thead::isNone() const { return getType() == TheadType::NONE; }
bool Thead::isTheadBinary() const { return getType() == TheadType::BINARY; }
bool Thead::isTheadStandard() const { return getType() == TheadType::STANDARD; }

TheadType Thead::getTypeRaw() const { return type; }

const Entity& Thead::getEntityRaw() const { assert(getTypeRaw() == TheadType::ENTITY); return ent; }
const TemplateHeadBinary& Thead::getTheadBinaryRaw() const { assert(getTypeRaw() == TheadType::BINARY); return *theadBinary; }
const TemplateHeadStandard& Thead::getTheadStandardRaw() const { assert(getTypeRaw() == TheadType::STANDARD); return *theadStandard; }

Entity& Thead::getEntityRaw() { assert(getTypeRaw() == TheadType::ENTITY); return ent; }
TemplateHeadBinary& Thead::getTheadBinaryRaw() { assert(getTypeRaw() == TheadType::BINARY); return *theadBinary; }
TemplateHeadStandard& Thead::getTheadStandardRaw() { assert(getTypeRaw() == TheadType::STANDARD); return *theadStandard; }

TheadOptions Thead::getOptions() const { return options; }