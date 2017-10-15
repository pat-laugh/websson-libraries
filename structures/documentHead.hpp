//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <memory>
#include <vector>

#include "base.hpp"
#include "entity.hpp"
#include "namespace.hpp"
#include "templateHead.hpp"
#include "webss.hpp"

namespace webss
{
	class ImportedDocument
	{
	public:
		ImportedDocument(std::string link);

		bool operator==(const ImportedDocument& o) const;
		bool operator!=(const ImportedDocument& o) const;

		const std::string& getLink() const;

	private:
		std::string link;
	};
	
	class ParamDocument
	{
	public:
		enum class Type
		{
			NONE, ENTITY_ABSTRACT, ENTITY_CONCRETE,
			EXPAND, IMPORT
		};

		ParamDocument();
		static ParamDocument makeEntityAbstract(Entity ent) { return ParamDocument(ent, Type::ENTITY_ABSTRACT); }
		static ParamDocument makeEntityConcrete(Entity ent) { return ParamDocument(ent, Type::ENTITY_CONCRETE); }
		static ParamDocument makeExpand(Entity ent) { assert(ent.getContent().isNamespace()); return ParamDocument(ent, Type::EXPAND); }
		ParamDocument(ImportedDocument import);
		~ParamDocument();

		ParamDocument(ParamDocument&& o);
		ParamDocument(const ParamDocument& o);

		ParamDocument& operator=(ParamDocument o);

		bool operator==(const ParamDocument& o) const;
		bool operator!=(const ParamDocument& o) const;

		Type getType() const;
		bool hasNamespace() const;
		const Entity& getEntity() const;
		const std::vector<Entity>& getEntityList() const;
		const Namespace& getNamespace() const;
		const ImportedDocument& getImport() const;

	private:
		Type type = Type::NONE;
		Entity ent;
		std::unique_ptr<ImportedDocument> import;

		ParamDocument(Entity ent, Type type);
		ParamDocument(Entity ent, Type type, ImportedDocument import);

		void destroyUnion();
		void copyUnion(ParamDocument&& o);
		void copyUnion(const ParamDocument& o);
	};
}