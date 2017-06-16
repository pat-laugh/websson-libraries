//MIT License
//Copyright 2017 Patrick Laughrea
#include "paramStandard.hpp"

#include "utils.hpp"

using namespace std;
using namespace webss;

ParamStandard::ParamStandard() {}
ParamStandard::ParamStandard(Webss defaultValue) : defaultValue(new Webss(move(defaultValue))) {}
ParamStandard::~ParamStandard() { destroyUnion(); }

ParamStandard::ParamStandard(ParamStandard&& o) { copyUnion(move(o)); }
ParamStandard::ParamStandard(const ParamStandard& o) { copyUnion(o); }

ParamStandard& ParamStandard::operator=(ParamStandard o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}

bool ParamStandard::operator==(const ParamStandard& o) const { return (this == &o) || (equalPtrs(thead, o.thead) && equalPtrs(defaultValue, o.defaultValue)); }
bool ParamStandard::operator!=(const ParamStandard& o) const { return !(*this == o); }

bool ParamStandard::hasDefaultValue() const { return defaultValue != nullptr; }

const Webss& ParamStandard::getDefaultValue() const
{
	assert(hasDefaultValue());
	return *defaultValue;
}
const shared_ptr<Webss>& ParamStandard::getDefaultPointer() const
{
	assert(hasDefaultValue());
	return defaultValue;
}

bool ParamStandard::hasThead() const { return thead != nullptr; }

const Thead& ParamStandard::getThead() const
{
	assert(hasThead());
	return *thead;
}

TypeThead ParamStandard::getTypeThead() const
{
	assert(hasThead());
	return thead->getType();
}

const TemplateHeadStandard& ParamStandard::getTheadStd() const
{
	assert(hasThead() && thead->isTheadStandard());
	return thead->getTheadStandard();
}

const TemplateHeadBinary& ParamStandard::getTheadBin() const
{
	assert(hasThead() && thead->isTheadBinary());
	return thead->getTheadBinary();
}

bool ParamStandard::isTextThead() const
{
	assert(hasThead());
	return thead->isText();
}
bool ParamStandard::isPlusThead() const
{
	assert(hasThead());
	return thead->isPlus();
}

void ParamStandard::setThead(Thead o)
{
	assert(!hasThead());
	thead.reset(new Thead(move(o)));
}

void ParamStandard::removeThead()
{
	assert(hasThead());
	thead.reset();
}

void ParamStandard::destroyUnion()
{
	removeThead();
}

void ParamStandard::copyUnion(ParamStandard&& o)
{
	thead = move(o.thead);
	defaultValue = move(o.defaultValue);
}
void ParamStandard::copyUnion(const ParamStandard& o)
{
	thead = unique_ptr<Thead>(new Thead(*o.thead));
	defaultValue = o.defaultValue;
}