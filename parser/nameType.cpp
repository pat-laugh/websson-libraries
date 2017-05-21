//MIT License
//Copyright 2017 Patrick Laughrea
#include "nameType.hpp"

#include "utilsSweepers.hpp"

using namespace std;
using namespace webss;

NameType::NameType(string&& name) : type(Type::NAME), name(std::move(name)) {}
NameType::NameType(Keyword keyword) : type(Type::KEYWORD), keyword(keyword) {}
NameType::NameType(const Entity& entity) : type(entity.getContent().isAbstract() ? Type::ENTITY_ABSTRACT : Type::ENTITY_CONCRETE), entity(entity) {}

NameType webss::parseNameType(SmartIterator& it, EntityManager& ents)
{
	string name = parseName(it);
	if (isKeyword(name))
		return{ Keyword(name) };
	else if (!ents.hasEntity(name))
		return{ move(name) };

	const Entity* ent = &ents[name];
scopeLoop:
	if (getTag(it) != Tag::SCOPE)
		return{ *ent };
	try
	{
		skipJunkToTag(++it, Tag::NAME_START);
		const auto& content = ent->getContent();
		if (content.isEnum())
			return content.getEnum().at(parseName(it));
		ent = &content.getNamespace().at(parseName(it));
		goto scopeLoop;
	}
	catch (const exception&) { throw runtime_error("could not get scoped value"); }
}