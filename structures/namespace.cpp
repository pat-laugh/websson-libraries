//MIT License
//Copyright 2017 Patrick Laughrea
#include "webss.hpp"

#include <cassert>

#include "utils.hpp"

using namespace std;
using namespace webss;

struct Namespace::NamespaceBody
{
	string name;
	Namespaces nspaces;
	Data data;
	Keymap keys;

	NamespaceBody(string&& name) : name(move(name)) {}
	NamespaceBody(string&& name, const Namespace& previousNspace) : name(move(name)), nspaces(previousNspace.getNamespaces())
	{
		nspaces.push_back(previousNspace);
	}

	bool operator==(const NamespaceBody& o) const
	{
		if (this == &o)
			return true;

		if (name != o.name || keys != o.keys)
			return false;

		//check data; only the content of entities must be checked
		//since they have a circular dependency with the namespace
		//the size of data and name of entites is checked in keys equality
		for (Data::size_type i = 0; i < data.size(); ++i)
			if (data[i].getContent() != o.data[i].getContent())
				return false;

		//don't compare previous namespaces as only the content of current namespaces matter
		return true;
	}
	bool operator!=(const NamespaceBody& o) const { return !(*this == o); }
};

Namespace::Namespace() {}
Namespace::Namespace(string name) : ptrBody(new NamespaceBody(move(name))) {}
Namespace::Namespace(string name, const Namespace& previousNspace) : ptrBody(new NamespaceBody(move(name), previousNspace)) {}

bool Namespace::empty() const { return getData().empty(); }
Namespace::size_type Namespace::size() const { return getData().size(); }

void Namespace::add(string key, Webss value) { add(Entity(move(key), move(value))); }
void Namespace::addSafe(string key, Webss value) { addSafe(Entity(move(key), move(value))); }

void Namespace::add(Entity ent)
{
	ent.setNamespace(*this);
	containerAddUnsafe(getKeys(), string(ent.getName()), size());
	getData().push_back(move(ent));
}
void Namespace::addSafe(Entity ent)
{
	ent.setNamespace(*this);
	containerAddSafe(getKeys(), string(ent.getName()), size());
	getData().push_back(move(ent));
}

bool Namespace::has(const string& key) const { return getKeys().find(key) != getKeys().end(); }

bool Namespace::operator==(const Namespace& o) const { return equalPtrs(ptrBody, o.ptrBody); }
bool Namespace::operator!=(const Namespace& o) const { return !(*this == o); }

Entity& Namespace::operator[](const string& key) { return getData()[accessKeyUnsafe<Keymap, size_type>(getKeys(), key)]; }
const Entity& Namespace::operator[](const string& key) const { return getData()[accessKeyUnsafe<Keymap, size_type>(getKeys(), key)]; }
Entity& Namespace::at(const string& key) { return getData()[accessKeySafe<Keymap, size_type>(getKeys(), key)]; }
const Entity& Namespace::at(const string& key) const { return getData()[accessKeySafe<Keymap, size_type>(getKeys(), key)]; }

const string& Namespace::getName() const { assert(hasBody()); return getBody().name; }
const Namespace::Namespaces& Namespace::getNamespaces() const { assert(hasBody()); return getBody().nspaces; }

Namespace::iterator Namespace::begin() { return getData().begin(); }
Namespace::iterator Namespace::end() { return getData().end(); }
Namespace::const_iterator Namespace::begin() const { return getData().begin(); }
Namespace::const_iterator Namespace::end() const { return getData().end(); }

bool Namespace::hasBody() const { return ptrBody != nullptr; }

Namespace::NamespaceBody& Namespace::getBody() { assert(hasBody()); return *ptrBody; }
const Namespace::NamespaceBody& Namespace::getBody() const { assert(hasBody()); return *ptrBody; }
Namespace::Data& Namespace::getData() { return getBody().data; }
const Namespace::Data& Namespace::getData() const { return getBody().data; }
Namespace::Keymap& Namespace::getKeys() { return getBody().keys; }
const Namespace::Keymap& Namespace::getKeys() const { return getBody().keys; }