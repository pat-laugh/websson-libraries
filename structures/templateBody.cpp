//MIT License
//Copyright 2017 Patrick Laughrea
#include "templateBody.hpp"

using namespace std;
using namespace webss;

TemplateBody::TemplateBody(Dictionary dict) : body(std::move(dict)) {}
TemplateBody::TemplateBody(List list) : body(std::move(list)) {}
TemplateBody::TemplateBody(Tuple tuple) : body(std::move(tuple)) {}
TemplateBody::TemplateBody(Tuple tuple, WebssType type) : body(std::move(tuple), type)
{
	assert(type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT);
}

WebssType TemplateBody::getType() const { return body.getTypeRaw(); }

const Webss& TemplateBody::getWebss() const { return body; }
const Dictionary& TemplateBody::getDictionary() const { return body.getDictionaryRaw(); }
const List& TemplateBody::getList() const { return body.getListRaw(); }
const Tuple& TemplateBody::getTuple() const { return body.getTupleRaw(); }

bool TemplateBody::isDictionary() const { return body.isDictionary(); }
bool TemplateBody::isList() const { return body.isList(); }
bool TemplateBody::isTuple() const { return body.isTuple(); }
bool TemplateBody::isTupleText() const { return body.isTupleText(); }

bool TemplateBody::operator==(const TemplateBody& o) const { return body == o.body; }
bool TemplateBody::operator!=(const TemplateBody& o) const { return !(*this == o); }

const Webss& TemplateBody::operator[](int index) const { return body[index]; }
const Webss& TemplateBody::at(int index) const { return body.at(index); }

const Webss& TemplateBody::operator[](const std::string& key) const { return body[key]; }
const Webss& TemplateBody::at(const std::string& key) const { return body.at(key); }