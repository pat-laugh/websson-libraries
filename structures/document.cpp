//MIT License
//Copyright 2017 Patrick Laughrea
#include "document.hpp"

using namespace std;
using namespace webss;

DocumentHead& Document::getHead() { return head; }
const DocumentHead& Document::getHead() const { return head; }

Tuple& Document::getBody() { return body; }
const Tuple& Document::getBody() const { return body; }

bool Document::operator==(const Document& o) const { return (this == &o) || (head == o.head && body == o.body); }
bool Document::operator!=(const Document& o) const { return !(*this == o); }

void Document::addHead(ParamDocument param)
{
	alternate.push_back(Alternate::HEAD);
	head.push_back(move(param));
}

void Document::addBody(Webss value)
{
	alternate.push_back(Alternate::BODY);
	body.add(move(value));
}

void Document::addBody(std::string key, Webss value)
{
	alternate.push_back(Alternate::BODY);
	body.add(move(key), move(value));
}

void Document::addBodySafe(std::string key, Webss value)
{
	alternate.push_back(Alternate::BODY);
	body.addSafe(move(key), move(value));
}

void Document::addImport(Webss value)
{
	alternate.push_back(Alternate::IMPORT);
	body.add(move(value));
}

void Document::addImportSafe(std::string key, Webss value)
{
	alternate.push_back(Alternate::IMPORT);
	body.addSafe(move(key), move(value));
}

const Document::AlternateVector& Document::getAlternate() const
{
	return alternate;
}