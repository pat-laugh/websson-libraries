#pragma once

#include <vector>

#include "importManager.h"
#include "parser.h"

namespace webss
{
	class ParamDocumentIncluder
	{
	private:
		EntityManager& ents;
		std::vector<Entity> entitiesToReAdd;
		std::vector<Entity> entitiesToRemove;

		void remove(const Entity& ent);
		void include(const Entity& ent);

	public:
		ParamDocumentIncluder(EntityManager& ents, const TemplateHeadScoped::Parameters& params);
		~ParamDocumentIncluder();

		void includeEntities(const ParamDocument& paramDoc);

		static bool namespacePresentScope(const EntityManager& ents, const Namespace& nspace)
		{
			const auto& name = nspace.getName();
			if (!ents.hasEntity(name))
				return false;

			//make sure they're the exact same entity, not just two different entities with the same name
			const auto& content = ents[name].getContent();
			return content.isNamespace() && content.getNamespaceSafe() == nspace;
		}
	};
}