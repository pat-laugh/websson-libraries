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
	Namespace nspace;

	bool operator==(const EntityBody& o) const { return (this == &o) || (name == o.name && content == o.content && nspace == o.nspace); }
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

bool Entity::operator==(const Entity& o) const { return equalPtrs(ptr, o.ptr); }
bool Entity::operator!=(const Entity& o) const { return !(*this == o); }

bool Entity::hasNamespace() const
{
	assert(hasBody());
	return ptr->nspace.hasBody();
}
const Namespace& Entity::getNamespace() const
{
	assert(hasNamespace());
	return ptr->nspace;
}
void Entity::setNamespace(const Namespace& nspace)
{
	assert(hasBody() && !hasNamespace());
	ptr->nspace = nspace;
}