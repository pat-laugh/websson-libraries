//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include <vector>

#include "base.h"
#include "entity.h"
#include "namespace.h"
#include "templateHead.h"
#include "webss.h"

namespace webss
{
	class ImportedDocument
	{
	public:
		ImportedDocument(Webss&& name);

		const Webss& getName() const;
		const std::string& getLink() const;
	private:
		Webss name;
	};
	
	struct ScopedDocument
	{
		BasicTemplateHead<ParamDocument> head;
		DocumentHead body;
	};
	
	class ParamDocument
	{
	public:
		enum class Type
		{
			NONE, ENTITY_ABSTRACT, ENTITY_CONCRETE, SCOPED_DOCUMENT,
			USING_ALL, USING_ONE, IMPORT
		};

		ParamDocument();
		static ParamDocument makeEntityAbstract(Entity ent) { return ParamDocument(ent, Type::ENTITY_ABSTRACT); }
		static ParamDocument makeEntityConcrete(Entity ent) { return ParamDocument(ent, Type::ENTITY_CONCRETE); }
		static ParamDocument makeUsingAll(Entity ent) { assert(ent.getContent().isNamespace()); return ParamDocument(ent, Type::USING_ALL); }
		static ParamDocument makeUsingOne(Entity ent, ImportedDocument import) { return ParamDocument(ent, Type::USING_ONE, std::move(import)); }
		ParamDocument(ImportedDocument import);
		ParamDocument(ScopedDocument scopedDoc);
		~ParamDocument();

		ParamDocument(ParamDocument&& o);
		ParamDocument(const ParamDocument& o);

		ParamDocument& operator=(ParamDocument&& o);
		ParamDocument& operator=(const ParamDocument& o);

		Type getType() const;
		bool hasNamespace() const;
		const Entity& getEntity() const;
		const Namespace& getNamespace() const;
		const ImportedDocument& getImport() const;
		const ScopedDocument& getScopedDoc() const;
	private:
		Type type = Type::NONE;
		union
		{
			Entity ent;
			ScopedDocument* scopedDoc;
		};

		ImportedDocument* import;

		ParamDocument(Entity ent, Type type);
		ParamDocument(Entity ent, Type type, ImportedDocument import);

		void destroyUnion();

		void copyUnion(ParamDocument&& o);

		void copyUnion(const ParamDocument& o);
	};
}