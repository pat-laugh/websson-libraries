//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "webss.h"

#include <assert.h>

using namespace std;
using namespace webss;

#define hasBody (ptr.get() != nullptr)

struct Entity::EntityBody
{
	string name;
	Webss content;
	shared_ptr<Namespace> nspace;
};

Entity::Entity() {}
Entity::Entity(string name, Webss content) : ptr(new EntityBody{ move(name), move(content) }) {}

const string& Entity::getName() const
{
	assert(hasBody);
	return ptr->name;
}
const Webss& Entity::getContent() const
{
	assert(hasBody);
	return ptr->content;
}

bool Entity::operator==(const Entity& o) const { return ptr == o.ptr; }

bool Entity::hasNamespace() const
{
	assert(hasBody);
	return ptr->nspace.get() != nullptr;
}
const Namespace& Entity::getNamespace() const
{
	assert(hasNamespace());
	return *ptr->nspace;
}
void Entity::setNamespace(const shared_ptr<Namespace>& nspace)
{
	assert(hasBody && !hasNamespace());
	ptr->nspace = nspace;
}