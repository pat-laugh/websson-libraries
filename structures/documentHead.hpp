//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

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
		ImportedDocument(Webss&& data);

		bool operator==(const ImportedDocument& o) const;
		bool operator!=(const ImportedDocument& o) const;

		const Webss& getData() const;
		const std::string& getLink() const;
	private:
		Webss data;
	};
	
	class ParamDocument
	{
	public:
		enum class Type
		{
			NONE, ENTITY_ABSTRACT, ENTITY_CONCRETE,
			EXPAND, SCOPED_IMPORT, SCOPED_IMPORT_LIST, IMPORT
		};

		ParamDocument();
		static ParamDocument makeEntityAbstract(Entity ent) { return ParamDocument(ent, Type::ENTITY_ABSTRACT); }
		static ParamDocument makeEntityConcrete(Entity ent) { return ParamDocument(ent, Type::ENTITY_CONCRETE); }
		static ParamDocument makeExpand(Entity ent) { assert(ent.getContent().isNamespace()); return ParamDocument(ent, Type::EXPAND); }
		static ParamDocument makeScopedImport(Entity ent, ImportedDocument import) { return ParamDocument(ent, Type::SCOPED_IMPORT, std::move(import)); }
		static ParamDocument makeScopedImport(std::vector<Entity> entList, ImportedDocument import) { return ParamDocument(std::move(entList), std::move(import)); }
		ParamDocument(ImportedDocument import);
		~ParamDocument();

		ParamDocument(ParamDocument&& o);
		ParamDocument(const ParamDocument& o);

		ParamDocument& operator=(ParamDocument&& o);
		ParamDocument& operator=(const ParamDocument& o);

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
		union
		{
			Entity ent;
			std::vector<Entity>* entList;
		};
		ImportedDocument* import = nullptr;

		ParamDocument(Entity ent, Type type);
		ParamDocument(Entity ent, Type type, ImportedDocument import);
		ParamDocument(std::vector<Entity> entList, ImportedDocument import);

		void destroyUnion();

		void copyUnion(ParamDocument&& o);

		void copyUnion(const ParamDocument& o);
	};
}