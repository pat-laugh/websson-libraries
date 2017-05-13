//MIT License
//Copyright 2017 Patrick Laughrea
#include "templateBody.hpp"

using namespace std;
using namespace webss;

TemplateBody::TemplateBody(Dictionary&& dict) : body(std::move(dict)) {}
TemplateBody::TemplateBody(List&& list) : body(std::move(list)) {}
TemplateBody::TemplateBody(Tuple&& tuple) : body(std::move(tuple)) {}
TemplateBody::TemplateBody(Tuple&& tuple, bool) : body(std::move(tuple), true) {}
TemplateBody::TemplateBody(const Dictionary& dict) : body(dict) {}
TemplateBody::TemplateBody(const List& list) : body(list) {}
TemplateBody::TemplateBody(const Tuple& tuple) : body(tuple) {}
TemplateBody::TemplateBody(const Tuple& tuple, bool) : body(tuple, true) {}

WebssType TemplateBody::getType() const { return body.getTypeRaw(); }

const Webss& TemplateBody::getWebss() const { return body; }
const Dictionary& TemplateBody::getDictionary() const { return body.getDictionaryRaw(); }
const List& TemplateBody::getList() const { return body.getListRaw(); }
const Tuple& TemplateBody::getTuple() const { return body.getTupleRaw(); }

bool TemplateBody::isDictionary() const { return body.isDictionary(); }
bool TemplateBody::isList() const { return body.isList(); }
bool TemplateBody::isTuple() const { return body.isTuple(); }
bool TemplateBody::isTupleText() const { return body.isTupleText(); }

const Webss& TemplateBody::operator[](int index) const { return body[index]; }
const Webss& TemplateBody::at(int index) const { return body.at(index); }

const Webss& TemplateBody::operator[](const std::string& key) const { return body[key]; }
const Webss& TemplateBody::at(const std::string& key) const { return body.at(key); }