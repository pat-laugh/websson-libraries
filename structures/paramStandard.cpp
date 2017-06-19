//MIT License
//Copyright 2017 Patrick Laughrea
#include "paramStandard.hpp"

#include "utils.hpp"

using namespace std;
using namespace webss;

ParamStd::ParamStd() {}
ParamStd::ParamStd(Webss defaultValue) : defaultValue(new Webss(move(defaultValue))) {}

ParamStd::ParamStd(ParamStd&& o) : thead(move(o.thead)), defaultValue(move(o.defaultValue)) {}
ParamStd::ParamStd(const ParamStd& o) : thead(o.hasThead() ? new Thead(*o.thead) : nullptr), defaultValue(o.defaultValue) {}

ParamStd& ParamStd::operator=(ParamStd o)
{
	thead = move(o.thead);
	defaultValue = move(o.defaultValue);
	return *this;
}

bool ParamStd::operator==(const ParamStd& o) const { return (this == &o) || (equalPtrs(thead, o.thead) && equalPtrs(defaultValue, o.defaultValue)); }
bool ParamStd::operator!=(const ParamStd& o) const { return !(*this == o); }

bool ParamStd::hasDefaultValue() const { return defaultValue != nullptr; }

const Webss& ParamStd::getDefaultValue() const
{
	assert(hasDefaultValue());
	return *defaultValue;
}
const shared_ptr<Webss>& ParamStd::getDefaultPointer() const
{
	assert(hasDefaultValue());
	return defaultValue;
}

bool ParamStd::hasThead() const { return thead != nullptr; }

const Thead& ParamStd::getThead() const
{
	assert(hasThead());
	return *thead;
}

TypeThead ParamStd::getTypeThead() const
{
	assert(hasThead());
	return thead->getType();
}

const TheadStd& ParamStd::getTheadStd() const
{
	assert(hasThead() && thead->isTheadStd());
	return thead->getTheadStd();
}

const TheadBin& ParamStd::getTheadBin() const
{
	assert(hasThead() && thead->isTheadBin());
	return thead->getTheadBin();
}

bool ParamStd::isTextThead() const
{
	assert(hasThead());
	return thead->isText();
}
bool ParamStd::isPlusThead() const
{
	assert(hasThead());
	return thead->isPlus();
}

void ParamStd::setThead(Thead o)
{
	assert(!hasThead());
	thead.reset(new Thead(move(o)));
}

void ParamStd::removeThead()
{
	assert(hasThead());
	thead.reset();
}