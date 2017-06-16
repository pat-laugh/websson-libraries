//MIT License
//Copyright 2017 Patrick Laughrea
#include "paramStandard.hpp"

#include "utils.hpp"

using namespace std;
using namespace webss;

ParamStandard::ParamStandard() {}
ParamStandard::ParamStandard(Webss&& webss) : defaultValue(new Webss(move(webss))) {}
ParamStandard::~ParamStandard() { destroyUnion(); }

ParamStandard::ParamStandard(ParamStandard&& o) { copyUnion(move(o)); }
ParamStandard::ParamStandard(const ParamStandard& o) { copyUnion(o); }

ParamStandard& ParamStandard::operator=(ParamStandard&& o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}
ParamStandard& ParamStandard::operator=(const ParamStandard& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(o);
	}
	return *this;
}

bool ParamStandard::operator==(const ParamStandard& o) const
{
	if (this == &o)
		return true;
	if (typeThead != o.typeThead || !equalPtrs(defaultValue, o.defaultValue))
		return false;
	switch (o.typeThead)
	{
	case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
		return true;
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
		return *theadBin == *o.theadBin; 
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		return *theadStd == *o.theadStd;
	default:
		assert(false); throw domain_error("");
	}
}
bool ParamStandard::operator!=(const ParamStandard& o) const { return !(*this == o); }

bool ParamStandard::hasDefaultValue() const { return defaultValue != nullptr; }
bool ParamStandard::hasTemplateHead() const { return typeThead != WebssType::NONE; }

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

WebssType ParamStandard::getTypeTemplateHead() const
{
	return typeThead;
}

const TemplateHeadBinary& ParamStandard::getTemplateHeadBinary() const
{
	assert(typeThead == WebssType::TEMPLATE_HEAD_BINARY || typeThead == WebssType::TEMPLATE_HEAD_PLUS_BINARY);
	return *theadBin;
}
const TemplateHeadStandard& ParamStandard::getTemplateHeadStandard() const
{
	assert(typeThead == WebssType::TEMPLATE_HEAD_STANDARD || typeThead == WebssType::TEMPLATE_HEAD_TEXT || typeThead == WebssType::TEMPLATE_HEAD_PLUS_STANDARD || typeThead == WebssType::TEMPLATE_HEAD_PLUS_TEXT);
	return *theadStd;
}

void ParamStandard::removeTemplateHead()
{
	switch (typeThead)
	{
	case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
		break;
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
		delete theadBin;
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		delete theadStd;
		break;
	default:
		assert(false);
	}
	typeThead = WebssType::NONE;
}
void ParamStandard::setTemplateHead(TemplateHeadBinary&& o, WebssType type)
{
	assert(!hasTemplateHead());
	theadBin = new TemplateHeadBinary(move(o));
	typeThead = type;
}
void ParamStandard::setTemplateHead(TemplateHeadStandard&& o, WebssType type)
{
	assert(!hasTemplateHead() && (type == WebssType::TEMPLATE_HEAD_STANDARD || type == WebssType::TEMPLATE_HEAD_TEXT));
	theadStd = new TemplateHeadStandard(move(o));
	typeThead = type;
}
void ParamStandard::setTemplateHead(TemplateHeadSelf)
{
	assert(!hasTemplateHead());
	typeThead = WebssType::TEMPLATE_HEAD_SELF;
}

void ParamStandard::destroyUnion()
{
	removeTemplateHead();
}

void ParamStandard::copyUnion(ParamStandard&& o)
{
	switch (o.typeThead)
	{
	case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
		break;
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
		theadBin = o.theadBin;
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		theadStd = o.theadStd;
		break;
	default:
		assert(false);
	}
	typeThead = o.typeThead;
	o.typeThead = WebssType::NONE;
	defaultValue = move(o.defaultValue);
}
void ParamStandard::copyUnion(const ParamStandard& o)
{
	switch (o.typeThead)
	{
	case WebssType::NONE: case WebssType::TEMPLATE_HEAD_SELF:
		break;
	case WebssType::TEMPLATE_HEAD_BINARY: case WebssType::TEMPLATE_HEAD_PLUS_BINARY:
		theadBin = new TemplateHeadBinary(*o.theadBin);
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD: case WebssType::TEMPLATE_HEAD_TEXT: case WebssType::TEMPLATE_HEAD_PLUS_STANDARD: case WebssType::TEMPLATE_HEAD_PLUS_TEXT:
		theadStd = new TemplateHeadStandard(*o.theadStd);
		break;
	default:
		assert(false);
	}
	typeThead = o.typeThead;
	defaultValue = o.defaultValue;
}