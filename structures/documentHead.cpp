//MIT License
//Copyright 2017 Patrick Laughrea
#include "documentHead.hpp"

#include "tuple.hpp"
#include "utils.hpp"

using namespace std;
using namespace webss;

ImportedDocument::ImportedDocument(string link) : link(move(link)) {}

bool ImportedDocument::operator==(const ImportedDocument& o) const { return link == o.link; }
bool ImportedDocument::operator!=(const ImportedDocument& o) const { return !(*this == o); }

const string& ImportedDocument::getLink() const { return link; }

ParamDocument::ParamDocument() {}
ParamDocument::ParamDocument(ImportedDocument import) : type(Type::IMPORT), import(new ImportedDocument(move(import))) {}
ParamDocument::~ParamDocument() { destroyUnion(); }

ParamDocument::ParamDocument(ParamDocument&& o) { copyUnion(move(o)); }
ParamDocument::ParamDocument(const ParamDocument& o) { copyUnion(o); }

ParamDocument& ParamDocument::operator=(ParamDocument o)
{
	destroyUnion();
	copyUnion(move(o));
	return *this;
}

bool ParamDocument::operator==(const ParamDocument& o) const
{
	if (this == &o)
		return true;
	if (type != o.type || !equalPtrs(import, o.import))
		return false;
	switch (type)
	{
	case Type::NONE: case Type::IMPORT:
		return true;
	case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::EXPAND:
		return ent == o.ent;
	default:
		assert(false); throw domain_error("");
	}
}
bool ParamDocument::operator!=(const ParamDocument& o) const { return !(*this == o); }

ParamDocument::Type ParamDocument::getType() const { return type; }
bool ParamDocument::hasNamespace() const { return type == Type::EXPAND; }
const Entity& ParamDocument::getEntity() const { return ent; }
const Namespace& ParamDocument::getNamespace() const { assert(hasNamespace()); return ent.getContent().getNamespace(); }
const ImportedDocument& ParamDocument::getImport() const { assert(import != nullptr); return *import; }

ParamDocument::ParamDocument(Entity ent, Type type) : type(type), ent(move(ent)) {}
ParamDocument::ParamDocument(Entity ent, Type type, ImportedDocument import) : type(type), ent(move(ent)), import(new ImportedDocument(move(import))) {}

void ParamDocument::destroyUnion()
{
	type = Type::NONE;
	import.reset();
}

void ParamDocument::copyUnion(ParamDocument&& o)
{
	ent = move(o.ent);
	type = o.type;
	import = move(o.import);
	o.type = Type::NONE;
}

void ParamDocument::copyUnion(const ParamDocument& o)
{
	ent = o.ent;
	type = o.type;
	if (o.import != nullptr)
		import.reset(new ImportedDocument(*o.import));
}