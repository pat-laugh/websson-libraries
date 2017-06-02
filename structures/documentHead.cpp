//MIT License
//Copyright 2017 Patrick Laughrea
#include "documentHead.hpp"

using namespace std;
using namespace webss;

ImportedDocument::ImportedDocument(Webss&& name) : name(move(name)) { assert(this->name.isString() && "import must reference a string"); }

const Webss& ImportedDocument::getName() const { return name; }
const string& ImportedDocument::getLink() const { return name.getString(); }


ParamDocument::ParamDocument() {}
ParamDocument::ParamDocument(ImportedDocument import) : type(Type::IMPORT), import(new ImportedDocument(move(import))) {}
ParamDocument::~ParamDocument() { destroyUnion(); }

ParamDocument::ParamDocument(ParamDocument&& o) { copyUnion(move(o)); }
ParamDocument::ParamDocument(const ParamDocument& o) { copyUnion(o); }

ParamDocument& ParamDocument::operator=(ParamDocument&& o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}
ParamDocument& ParamDocument::operator=(const ParamDocument& o)
{
	if (this != &o)
	{
		destroyUnion();
		copyUnion(o);
	}
	return *this;
}

ParamDocument::Type ParamDocument::getType() const { return type; }
bool ParamDocument::hasNamespace() const { return type == Type::EXPAND; }
const Entity& ParamDocument::getEntity() const { return ent; }
const Namespace& ParamDocument::getNamespace() const { return ent.getContent().getNamespace(); }
const ImportedDocument& ParamDocument::getImport() const { return *import; }

ParamDocument::ParamDocument(Entity ent, Type type) : type(type), ent(move(ent)) {}
ParamDocument::ParamDocument(Entity ent, Type type, ImportedDocument import) : type(type), ent(move(ent)), import(new ImportedDocument(move(import))) {}

void ParamDocument::destroyUnion()
{
	switch (type)
	{
	case Type::USING_ONE: case Type::IMPORT:
		delete import;
		break;
	default:
		break;
	}
	type = Type::NONE;
}

void ParamDocument::copyUnion(ParamDocument&& o)
{
	switch (o.type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND:
		ent = move(o.ent);
		break;
	case Type::USING_ONE:
		ent = move(o.ent);
		import = o.import;
		break;
	case Type::IMPORT:
		import = o.import;
		break;
	default:
		break;
	}

	type = o.type;
	o.type = Type::NONE;
}

void ParamDocument::copyUnion(const ParamDocument& o)
{
	switch (o.type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND:
		ent = o.ent;
		break;
	case Type::USING_ONE:
		ent = o.ent;
		import = new ImportedDocument(*o.import);
		break;
	case Type::IMPORT:
		import = new ImportedDocument(*o.import);
		break;
	default:
		break;
	}

	type = o.type;
}