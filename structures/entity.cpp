//MIT License
//Copyright 2017 Patrick Laughrea
#include "webss.hpp"

#include <cassert>

#include "utils.hpp"

using namespace std;
using namespace webss;

struct Entity::EntityBody
{
	string name;
	Webss content;

	bool operator==(const EntityBody& o) const { return content == o.content; }
	bool operator!=(const EntityBody& o) const { return !(*this == o); }
};

Entity::Entity() {}
Entity::Entity(string name, Webss content) : ptr(new EntityBody{ move(name), move(content) }) {}

bool Entity::hasBody() const
{
	return ptr != nullptr;
}
const string& Entity::getName() const
{
	assert(hasBody());
	return ptr->name;
}
const Webss& Entity::getContent() const
{
	assert(hasBody());
	return ptr->content;
}

bool Entity::operator==(const Entity& o) const { return (this == &o) || equalPtrs(ptr, o.ptr); }
bool Entity::operator!=(const Entity& o) const { return !(*this == o); }

bool Entity::hasNamespace() const { return !nspacePtr.expired(); }
const Namespace Entity::getNamespace() const { assert(hasNamespace()); return nspacePtr.lock(); }
void Entity::setNamespace(const Namespace& nspace) { nspacePtr = nspace.getBodyPointerWeak(); }
void Entity::removeNamespace() { nspacePtr.reset(); }
void Entity::setDocId(shared_ptr<string> docId) { this->docId = move(docId); }
const std::string& Entity::getDocId() const { return *docId; }
const std::string* Entity::getDocIdPtr() const { return docId.get(); };