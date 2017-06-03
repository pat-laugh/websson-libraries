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
const Entity& ParamDocument::getEntity() const { assert(type != Type::SCOPED_IMPORT_LIST); return ent; }
const std::vector<Entity>& ParamDocument::getEntityList() const { assert(type == Type::SCOPED_IMPORT_LIST); return *entList; }
const Namespace& ParamDocument::getNamespace() const { return ent.getContent().getNamespace(); }
const ImportedDocument& ParamDocument::getImport() const { return *import; }

ParamDocument::ParamDocument(Entity ent, Type type) : type(type), ent(move(ent)) {}
ParamDocument::ParamDocument(Entity ent, Type type, ImportedDocument import) : type(type), ent(move(ent)), import(new ImportedDocument(move(import))) {}
ParamDocument::ParamDocument(vector<Entity> entList, ImportedDocument import) : type(Type::SCOPED_IMPORT_LIST), entList(new vector<Entity>(move(entList))), import(new ImportedDocument(move(import))) {}

void ParamDocument::destroyUnion()
{
	switch (type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND: case Type::SCOPED_IMPORT:
		ent.~Entity();
		break;
	case Type::SCOPED_IMPORT_LIST:
		delete entList;
		break;
	default:
		break;
	}

	type = Type::NONE;
	if (import != nullptr)
	{
		delete import;
		import = nullptr;
	}
}

void ParamDocument::copyUnion(ParamDocument&& o)
{
	switch (o.type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND: case Type::SCOPED_IMPORT:
		new (&ent) Entity(move(o.ent));
		o.ent.~Entity();
		break;
	case Type::SCOPED_IMPORT_LIST:
		entList = o.entList;
		break;
	default:
		break;
	}

	type = o.type;
	import = o.import;
	o.type = Type::NONE;
}

void ParamDocument::copyUnion(const ParamDocument& o)
{
	switch (o.type)
	{
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND: case Type::SCOPED_IMPORT:
		new (&ent) Entity(o.ent);
		break;
	case Type::SCOPED_IMPORT_LIST:
		entList = new vector<Entity>(*o.entList);
		break;
	default:
		break;
	}

	type = o.type;
	if (o.import != nullptr)
		import = new ImportedDocument(*o.import);
}