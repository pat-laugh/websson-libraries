//MIT License
//Copyright 2017 Patrick Laughrea
#include "paramDocumentIncluder.hpp"

#include <stdexcept>

#include "parser.hpp"

using namespace std;
using namespace webss;

void ParamDocumentIncluder::remove(const Entity& ent)
{
	entitiesToReAdd.push_back(ent);
	ents.removeLocal(ent);
}

void ParamDocumentIncluder::include(const Entity& ent)
{
	entitiesToRemove.push_back(ent);
	ents.addLocalSafe(ent);
}

ParamDocumentIncluder::ParamDocumentIncluder(EntityManager& ents, const TemplateHeadScoped::Parameters& params) : ents(ents)
{
	for (const auto& param : params)
		includeEntities(param);
}

ParamDocumentIncluder::~ParamDocumentIncluder()
{
	for (const auto& ent : entitiesToRemove)
		ents.removeLocal(ent);

	for (const auto& ent : entitiesToReAdd)
		ents.addLocal(ent);
}

void ParamDocumentIncluder::includeEntities(const ParamDocument& paramDoc)
{
	using Type = ParamDocument::Type;
	switch (paramDoc.getType())
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE:
		include(paramDoc.getEntity());
		break;
	case Type::USING_ONE:
	{
		Entity ent(paramDoc.getEntity().getName(), paramDoc.getEntity().getContent());
		include(ent);
		break;
	}
	case Type::USING_ALL:
	{
		const auto& nspace = paramDoc.getNamespace();

		//first check the namespace entity is accessible; if so it has to be removed since
		//it'll no longer be necessary and an entity with the same name could be inside
		if (namespacePresentScope(ents, nspace))
			remove(ents[nspace.getName()]);

		for (const auto& ent : nspace)
			include(ent);
		break;
	}
	case Type::IMPORT:
	{
		const auto& link = paramDoc.getImport().getLink();
		for (const auto& entPair : ImportManager::getInstance().importDocument(link))
			include(entPair.second);
		break;
	}
	default:
		assert(false); throw domain_error("");
	}
}