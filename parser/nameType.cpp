//MIT License
//Copyright 2017 Patrick Laughrea
#include "nameType.hpp"

#include <cassert>

#include "utilsSweepers.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

NameType::NameType(string&& name) : type(Type::NAME), name(std::move(name)) {}
NameType::NameType(Keyword keyword) : type(Type::KEYWORD), keyword(keyword) {}
NameType::NameType(const Entity& entity) : type(entity.getContent().isAbstract() ? Type::ENTITY_ABSTRACT : Type::ENTITY_CONCRETE), entity(entity) {}

NameType webss::parseNameType(TagIterator& tagit, const EntityManager& ents)
{
	assert(tagit.getIt() && isNameStart(*tagit.getIt()));
	string name = parseName(tagit.getItSafe());
	if (isKeyword(name))
		return{ Keyword(name) };
	else if (!ents.hasEntity(name))
		return{ move(name) };

	const Entity* ent = &ents[name];
scopeLoop:
	if (tagit.update() != Tag::SCOPE)
		return{ *ent };
	try
	{
		(++tagit).sofertTag(Tag::NAME_START);
		const auto& content = ent->getContent();
		if (content.isEnum())
			return content.getEnum().at(parseName(tagit.getItSafe()));
		ent = &content.getNamespace().at(parseName(tagit.getIt()));
		goto scopeLoop;
	}
	catch (const exception&) { throw runtime_error("could not get scoped value"); }
}