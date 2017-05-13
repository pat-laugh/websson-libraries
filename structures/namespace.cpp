//MIT License
//Copyright(c) 2017 Patrick Laughrea
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
	PtrThis ptrThis;
};

Namespace::Namespace(string name) : ptrBody(new NamespaceBody{ move(name) })
{
	ptrBody->ptrThis = PtrThis(new Namespace(*this));
}

Namespace::Namespace(string name, const Namespace& previousNspace) : ptrBody(new NamespaceBody{ move(name), previousNspace.getNamespaces() })
{
	ptrBody->ptrThis = PtrThis(new Namespace(*this));
}

bool Namespace::empty() const
{
	return getData().empty();
}
Namespace::size_type Namespace::size() const
{
	return getData().size();
}

void Namespace::add(string key, Webss value) { add(Entity(move(key), move(value))); }
void Namespace::addSafe(string key, Webss value) { addSafe(Entity(move(key), move(value))); }

void Namespace::add(Entity&& ent)
{
	ent.setNamespace(getPointer());
	containerAddUnsafe(getKeys(), string(ent.getName()), size());
	getData().push_back(move(ent));
}
void Namespace::addSafe(Entity&& ent)
{
	ent.setNamespace(getPointer());
	containerAddSafe(getKeys(), string(ent.getName()), size());
	getData().push_back(move(ent));
}

bool Namespace::has(const string& key) const { return getKeys().find(key) != getKeys().end(); }

bool Namespace::operator==(const Namespace& o) const { return ptrBody == o.ptrBody; }

Entity& Namespace::operator[](const string& key) { return getData()[accessKeyUnsafe<Keymap, size_type>(getKeys(), key)]; }
const Entity& Namespace::operator[](const string& key) const { return getData()[accessKeyUnsafe<Keymap, size_type>(getKeys(), key)]; }
Entity& Namespace::at(const string& key) { return getData()[accessKeySafe<Keymap, size_type>(getKeys(), key)]; }
const Entity& Namespace::at(const string& key) const { return getData()[accessKeySafe<Keymap, size_type>(getKeys(), key)]; }

const string& Namespace::getName() const { return getBody().name; }
const Namespace::Namespaces& Namespace::getNamespaces() const { return getBody().nspaces; }
const Namespace::PtrThis& Namespace::getPointer() const { return getBody().ptrThis; }

Namespace::iterator Namespace::begin() { return getData().begin(); }
Namespace::iterator Namespace::end() { return getData().end(); }
Namespace::const_iterator Namespace::begin() const { return getData().begin(); }
Namespace::const_iterator Namespace::end() const { return getData().end(); }

bool Namespace::hasBody() const { return ptrBody.get() != nullptr; }

Namespace::NamespaceBody& Namespace::getBody() { assert(hasBody()); return *ptrBody; }
const Namespace::NamespaceBody& Namespace::getBody() const { assert(hasBody()); return *ptrBody; }
Namespace::Data& Namespace::getData() { return getBody().data; }
const Namespace::Data& Namespace::getData() const { return getBody().data; }
Namespace::Keymap& Namespace::getKeys() { return getBody().keys; }
const Namespace::Keymap& Namespace::getKeys() const { return getBody().keys; }

Namespace::Namespace() : ptrBody(new NamespaceBody{}) {}