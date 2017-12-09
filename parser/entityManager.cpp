//MIT License
//Copyright 2017 Patrick Laughrea
#include "entityManager.hpp"

#include "errors.hpp"
#include "structures/utils.hpp"

using namespace std;
using namespace webss;

static const char* ERROR_ENTITY_EXISTS = "entity already exists: ";

EntityManager::EntityManager() {}
EntityManager::~EntityManager() { clearAll(); }

const unordered_map<string, Entity>& EntityManager::getEnts() const
{
	return ents;
}

void EntityManager::clearAll()
{
	for (auto it = ents.begin(); it != ents.end();)
		it = ents.erase(it);
}

void EntityManager::addAlias(string name, Entity ent)
{
	ents.insert({ move(name), move(ent) });
}

void EntityManager::add(string name, Webss content)
{
	add(Entity(move(name), move(content)));
}

void EntityManager::addSafe(string name, Webss content)
{
	addSafe(Entity(move(name), move(content)));
}

void EntityManager::add(Entity ent)
{
	auto name = ent.getName();
	ents.insert({ move(name), move(ent) });
}

void EntityManager::addSafe(Entity ent)
{
	if (!hasEntity(ent.getName()))
		add(move(ent));
	else if (!equalPtrs(ent.getDocIdPtr(), operator[](ent.getName()).getDocIdPtr())) //else do nothing
		throw runtime_error(WEBSSON_EXCEPTION(ERROR_ENTITY_EXISTS + ent.getName()));
}

bool EntityManager::hasEntity(const string& s) const { return ents.find(s) != ents.end(); }

Entity& EntityManager::operator[](const string& name) { return ents.find(name)->second; }
const Entity& EntityManager::operator[](const string& name) const { return ents.find(name)->second; }

Entity& EntityManager::at(const string& name) { return ents.at(name); }
const Entity& EntityManager::at(const string& name) const { return ents.at(name); }

void EntityManager::remove(const string& name)
{
	ents.erase(name);
}

void EntityManager::remove(const Entity& ent)
{
	remove(ent.getName());
}