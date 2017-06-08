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

	bool operator==(const EntityBody& o) const { return (this == &o) || (name == o.name && content == o.content); }
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

bool Entity::operator==(const Entity& o) const { return (this == &o) || (equalPtrs(ptr, o.ptr) && nspace == o.nspace); }
bool Entity::operator!=(const Entity& o) const { return !(*this == o); }

bool Entity::hasNamespace() const
{
	return nspace.hasBody();
}
const Namespace& Entity::getNamespace() const
{
	assert(hasNamespace());
	return nspace;
}
void Entity::setNamespace(const Namespace& nspace)
{
	this->nspace = nspace;
}
void Entity::removeNamespace()
{
	nspace = Namespace();
}