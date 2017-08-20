//MIT License
//Copyright 2017 Patrick Laughrea
#include "entityManager.hpp"

using namespace std;
using namespace webss;

static const char* ERROR_ENTITY_EXISTS = "entity already exists: ";

EntityManager::EntityManager() {}
EntityManager::~EntityManager() { clearAll(); }

vector<Entity> EntityManager::getPublicEnts() const
{
	vector<Entity> ents;
	for (auto sPtr : publics)
		ents.push_back(privates.find(*sPtr)->second);
	return ents;
}

void EntityManager::clearAll()
{
	clearPublics();
	for (auto it = privates.begin(); it != privates.end();)
		it = privates.erase(it);
}

void EntityManager::clearPublics()
{
	for (auto it = publics.begin(); it != publics.end();)
		it = publics.erase(it);
}

void EntityManager::addPrivateEntity(string name, Entity ent)
{
	privates.insert({ move(name), move(ent) });
}

void EntityManager::addPrivate(string name, Webss content)
{
	addPrivate(Entity(move(name), move(content)));
}

void EntityManager::addPrivateSafe(string name, Webss content)
{
	addPrivateSafe(Entity(move(name), move(content)));
}

void EntityManager::addPrivate(Entity ent)
{
	const auto& name = ent.getName();
	privates.insert({ name, move(ent) });
}

void EntityManager::addPrivateSafe(Entity ent)
{
	if (hasEntity(ent.getName()))
		throw runtime_error(ERROR_ENTITY_EXISTS + ent.getName());

	addPrivate(move(ent));
}

void EntityManager::addPublic(string name, Webss content)
{
	addPublic(Entity(move(name), move(content)));
}

void EntityManager::addPublicSafe(string name, Webss content)
{
	addPublicSafe(Entity(move(name), move(content)));
}

void EntityManager::addPublic(Entity ent)
{
	publics.insert(const_cast<string*>(&ent.getName()));
	addPrivate(move(ent));
}

void EntityManager::addPublicSafe(Entity ent)
{
	if (!hasEntity(ent.getName()))
		addPublic(move(ent));
	else if (ent != operator[](ent.getName())) //else do nothing
		throw runtime_error(ERROR_ENTITY_EXISTS + ent.getName());
}

bool EntityManager::hasEntity(const string& s) const { return privates.find(s) != privates.end(); }

Entity& EntityManager::operator[](const string& name) { return privates.find(name)->second; }
const Entity& EntityManager::operator[](const string& name) const { return privates.find(name)->second; }

Entity& EntityManager::at(const string& name) { return privates.at(name); }
const Entity& EntityManager::at(const string& name) const { return privates.at(name); }

void EntityManager::removePrivate(const string& name)
{
	privates.erase(name);
}

void EntityManager::removePublic(const string& name)
{
	publics.erase(const_cast<string*>(&name));
	removePrivate(name);
}

void EntityManager::removePrivate(const Entity& ent)
{
	removePrivate(ent.getName());
}

void EntityManager::removePublic(const Entity& ent)
{
	removePublic(ent.getName());
}