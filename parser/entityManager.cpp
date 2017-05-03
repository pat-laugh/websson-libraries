//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "entityManager.h"

using namespace std;
using namespace webss;

const char* ERROR_ENTITY_EXISTS = "entity already exists: ";

EntityManager::EntityManager() {}
EntityManager::~EntityManager() { clearAll(); }

vector<Entity> EntityManager::getLocalEnts() const
{
	vector<Entity> ents;
	for (auto sPtr : locals)
		ents.push_back(globals.find(*sPtr)->second);
	return ents;
}

void EntityManager::clearAll()
{
	clearLocals();
	for (auto it = globals.begin(); it != globals.end();)
		it = globals.erase(it);
}

void EntityManager::clearLocals()
{
	for (auto it = locals.begin(); it != locals.end();)
		it = locals.erase(it);
}

void EntityManager::addGlobalEntity(string name, Entity ent)
{
	globals.insert({ move(name), move(ent) });
}

void EntityManager::addGlobal(string name, Webss content)
{
	addGlobal(Entity(move(name), move(content)));
}

void EntityManager::addGlobalSafe(string name, Webss content)
{
	addGlobalSafe(Entity(move(name), move(content)));
}

void EntityManager::addGlobal(Entity ent)
{
	const auto& name = ent.getName();
	globals.insert({ name, move(ent) });
}

void EntityManager::addGlobalSafe(Entity ent)
{
	if (hasEntity(ent.getName()))
		throw runtime_error(ERROR_ENTITY_EXISTS + ent.getName());

	addGlobal(move(ent));
}

void EntityManager::addLocal(string name, Webss content)
{
	addLocal(Entity(move(name), move(content)));
}

void EntityManager::addLocalSafe(string name, Webss content)
{
	addLocalSafe(Entity(move(name), move(content)));
}

void EntityManager::addLocal(Entity ent)
{
	locals.insert(const_cast<string*>(&ent.getName()));
	addGlobal(move(ent));
}

void EntityManager::addLocalSafe(Entity ent)
{
	if (hasEntity(ent.getName()))
		throw runtime_error(ERROR_ENTITY_EXISTS + ent.getName());

	addLocal(move(ent));
}

bool EntityManager::hasEntity(const string& s) const { return globals.find(s) != globals.end(); }

Entity& EntityManager::operator[](const string& name) { return globals.find(name)->second; }
const Entity& EntityManager::operator[](const string& name) const { return globals.find(name)->second; }

Entity& EntityManager::at(const string& name) { return globals.at(name); }
const Entity& EntityManager::at(const string& name) const { return globals.at(name); }

void EntityManager::removeGlobal(const string& name)
{
	globals.erase(name);
}

void EntityManager::removeLocal(const string& name)
{
	locals.erase(const_cast<string*>(&name));
	removeGlobal(name);
}

void EntityManager::removeGlobal(const Entity& ent)
{
	removeGlobal(ent.getName());
}

void EntityManager::removeLocal(const Entity& ent)
{
	removeLocal(ent.getName());
}