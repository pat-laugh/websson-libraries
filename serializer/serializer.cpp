//MIT License
//Copyright 2017 Patrick Laughrea
#include "serializer.hpp"

#include <cmath>
#include <cstdio>
#include <limits>
#include <type_traits>

#include "structures/templatePlus.hpp"
#include "utils/constants.hpp"
#include "utils/utils.hpp"
#include "utils/utilsWebss.hpp"

using namespace std;
using namespace webss;

template <ConType::Enum CON> void putContainerStart(StringBuilder&) { assert(false); }
template <> void putContainerStart<ConType::DOCUMENT>(StringBuilder&) {}
template <> void putContainerStart<ConType::DICTIONARY>(StringBuilder& out) { out += OPEN_DICTIONARY; }
template <> void putContainerStart<ConType::LIST>(StringBuilder& out) { out += OPEN_LIST; }
template <> void putContainerStart<ConType::TUPLE>(StringBuilder& out) { out += OPEN_TUPLE; }
template <> void putContainerStart<ConType::THEAD>(StringBuilder& out) { out += OPEN_TEMPLATE; }

template <ConType::Enum CON> void putContainerEnd(StringBuilder&) { assert(false); }
template <> void putContainerEnd<ConType::DOCUMENT>(StringBuilder&) {}
template <> void putContainerEnd<ConType::DICTIONARY>(StringBuilder& out) { out += CLOSE_DICTIONARY; }
template <> void putContainerEnd<ConType::LIST>(StringBuilder& out) { out += CLOSE_LIST; }
template <> void putContainerEnd<ConType::TUPLE>(StringBuilder& out) { out += CLOSE_TUPLE; }
template <> void putContainerEnd<ConType::THEAD>(StringBuilder& out) { out += CLOSE_TEMPLATE; }

template <ConType::Enum CON>
class ContainerIncluder
{
private:
	StringBuilder& out;
public:
	ContainerIncluder(StringBuilder& out) : out(out) { putContainerStart<CON>(out); }
	~ContainerIncluder() { putContainerEnd<CON>(out); }
};

template <class Container, ConType::Enum CON>
void putSeparatedValues(StringBuilder& out, const Container& cont, function<void(typename Container::const_iterator it)> output)
{
	ContainerIncluder<CON> includer(out);
	if (!cont.empty())
	{
		typename Container::const_iterator it = cont.begin();
		output(it);
		while (++it != cont.end())
		{
			out += CHAR_SEPARATOR;
			output(it);
		}
	}
}

class SerializerTemplate : public Serializer
{
public:
	void putTheadBin(StringBuilder& out, const TheadBin& thead)
	{
		putThead<TheadBin, ParamBin>(out, thead, [&](StringBuilder& out, const string& key, const ParamBin& param) { putParamBin(out, key, param); });
	}
	void putTheadStd(StringBuilder& out, const TheadStd& thead)
	{
		putThead<TheadStd, ParamStd>(out, thead, [&](StringBuilder& out, const string& key, const ParamStd& param) { putParamStd(out, key, param); });
	}
	void putTheadText(StringBuilder& out, const TheadStd& thead)
	{
		out += ASSIGN_CONTAINER_STRING;
		putThead<TheadStd, ParamStd>(out, thead, [&](StringBuilder& out, const string& key, const ParamStd& param) { putParamText(out, key, param); });
	}

	template <class Params>
	void putTemplateList(StringBuilder& out, const Params& params, const List& list, function<void(StringBuilder& out, const Params& params, const Tuple& tuple)>&& putTupleRegular, function<void(StringBuilder& out, const Params& params, const Tuple& tuple)>&& putTupleText)
	{
		static const auto CON = ConType::LIST;
		putSeparatedValues<List, CON>(out, list, [&](typename List::const_iterator it)
		{
			const auto type = it->getTypeRaw();
			if (type == WebssType::TUPLE)
				putTupleRegular(out, params, it->getTupleRaw());
			else
			{
				assert(type == WebssType::TUPLE_TEXT);
				putTupleText(out, params, it->getTupleRaw());
			}
		});
	}
private:
	template <class Thead, class Param>
	void putThead(StringBuilder& out, const Thead& thead, function<void(StringBuilder& out, const string& key, const Param& param)>&& putParam)
	{
		static const auto CON = ConType::THEAD;
		if (thead.hasEntity())
		{
			ContainerIncluder<CON> includer(out);
			out += CHAR_EXPAND;
			putEntityName(out, thead.getEntity());
		}
		else
		{
			auto&& keyValues = thead.getParams().getOrderedKeyValues();
			using Type = typename remove_reference<decltype(keyValues)>::type;
			putSeparatedValues<Type, CON>(out, keyValues, [&](typename Type::const_iterator it)
			{
				assert(it->first != nullptr && "can't have anonymous key in dictionary, template head or enum");
				putParam(out, *it->first, *it->second);
			});
		}
	}

	void putParamBin(StringBuilder& out, const string& key, const ParamBin& param)
	{
		const auto& bhead = param.getSizeHead();
		{
			ContainerIncluder<ConType::TUPLE> includer(out);
			using Type = ParamBin::SizeHead::Type;
			switch (bhead.getType())
			{
			case Type::EMPTY:
				break;
			case Type::KEYWORD:
				out += bhead.getKeyword().toString();
				break;
			case Type::NUMBER:
				out += to_string(bhead.size());
				break;
			case Type::THEAD:
				putTheadBin(out, bhead.getThead());
				break;
			case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER: case Type::ENTITY_THEAD:
				putEntityName(out, bhead.getEntity());
				break;
			case Type::SELF:
				putTheadSelf(out);
				break;
			default:
				assert(false);
			}
			putBinSizeList(out, param.getSizeList());
		}

		out += key;
		if (bhead.hasDefaultValue())
			putCharValue(out, bhead.getDefaultValue(), ConType::THEAD);
		else
			assert(!bhead.isTheadSelf());
	}
	void putParamStd(StringBuilder& out, const string& key, const ParamStd& param)
	{
		if (param.hasThead())
		{
			switch (param.getTypeThead())
			{
			case TypeThead::SELF:
				putTheadSelf(out);
				break;
			case TypeThead::BIN:
				putTheadBin(out, param.getTheadBin());
				break;
			case TypeThead::STD:
				if (param.getThead().isText())
					putTheadText(out, param.getTheadStd());
				else
					putTheadStd(out, param.getTheadStd());
				break;
			default:
				assert(false); break;
			}
		}

		out += key;
		if (param.hasDefaultValue())
			putCharValue(out, param.getDefaultValue(), ConType::THEAD);
		else
			assert(param.getTypeThead() != TypeThead::SELF);
	}
	void putParamText(StringBuilder& out, const string& key, const ParamStd& param)
	{
		out += key;
		if (param.hasDefaultValue())
		{
			const auto& webss = param.getDefaultValue();
			assert(webss.getType() == WebssType::PRIMITIVE_STRING && "template head text parameters' values can only be of type string");
			putCharValue(out, webss, ConType::THEAD);
		}
	}

	void putBinSizeList(StringBuilder& out, const ParamBin::SizeList& blist)
	{
		using Type = ParamBin::SizeList::Type;
		if (blist.isOne())
			return;

		ContainerIncluder<ConType::LIST> includer(out);
		switch (blist.getType())
		{
		case Type::EMPTY:
			break;
		case Type::NUMBER:
			out += to_string(blist.size());
			break;
		case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
			putEntityName(out, blist.getEntity());
			break;
		default:
			assert(false);
		}
	}
};

Serializer::Serializer() {}

void Serializer::putDocumentHead(StringBuilder& out, const DocumentHead& docHead)
{
	static const auto CON = ConType::DOCUMENT;
	putSeparatedValues<DocumentHead, CON>(out, docHead, [&](DocumentHead::const_iterator it)
	{
		using Type = decltype(it->getType());
		switch (it->getType())
		{
		case Type::ENTITY_ABSTRACT:
			putEntityAbstract(out, it->getEntity());
			break;
		case Type::ENTITY_CONCRETE:
			putEntityConcrete(out, it->getEntity(), CON);
			break;
		case Type::EXPAND:
			putExpandDocumentHead(out, it->getNamespace());
			break;
		case Type::IMPORT:
			putImport(out, it->getImport());
			break;
		case Type::SCOPED_IMPORT:
			putScopedImport(out, it->getEntity(), it->getImport());
			break;
		case Type::SCOPED_IMPORT_LIST:
			putScopedImportList(out, it->getEntityList(), it->getImport());
			break;
		default:
			assert(false);
		}
	});
}

void Serializer::putDocument(StringBuilder& out, const Document& doc)
{
	static const auto CON = ConType::DOCUMENT;

	if (doc.getHead().size() > 0)
	{
		putDocumentHead(out, doc.getHead());
		out += "\n\n";
	}

	using Type = decltype(doc.getOrderedKeyValues());
	putSeparatedValues<Type, CON>(out, doc.getOrderedKeyValues(), [&](Type::const_iterator it)
	{
		if (it->first == nullptr)
			putConcreteValue(out, *it->second, CON);
		else
			putExplicitKeyValue(out, *it->first, *it->second, CON);
	});
}

void Serializer::putPreviousNamespaceNames(StringBuilder& out, const Namespace& nspace)
{
	const auto& nspaces = nspace.getNamespaces();
	if (!nspaces.empty())
	{
		//check the list of namespaces in reverse order until there is an namespace that is in
		//the current scope, then put all the namespaces that were not in the current scope

		auto i = nspaces.size();
		while (i > 0 && !namespaceCurrentScope(nspaces[i - 1]))
			--i;

		while (i < nspaces.size())
			out += nspaces[i++].getName() + CHAR_SCOPE;
	}
}

bool Serializer::namespaceCurrentScope(const Namespace& nspace)
{
	return currentNamespaces.find(nspace.getPointerBody()) != currentNamespaces.end();
	return true;
}

void Serializer::putEntityNameWithoutNamespace(StringBuilder& out, const Entity& ent)
{
	out += ent.getName();
}

void Serializer::putEntityName(StringBuilder& out, const Entity& ent)
{
	if (ent.hasNamespace())
	{
		const auto& nspace = ent.getNamespace();
		if (!namespaceCurrentScope(nspace))
		{
			putPreviousNamespaceNames(out, nspace);
			out += nspace.getName() + CHAR_SCOPE;
		}
	}
	out += ent.getName();
}

void Serializer::putAbstractValue(StringBuilder& out, const Webss& webss)
{
	switch (webss.getTypeRaw())
	{
	case WebssType::THEAD:
	{
		const auto& thead = webss.getTheadRaw();
		if (thead.isPlus())
			out += CHAR_THEAD_PLUS;
		switch (thead.getTypeRaw())
		{
		case TypeThead::BIN:
			putTheadBin(out, thead.getTheadBinRaw());
			break;
		case TypeThead::STD:
			if (thead.isText())
				putTheadText(out, thead.getTheadStdRaw());
			else
				putTheadStd(out, thead.getTheadStdRaw());
			break;
		case TypeThead::ENTITY:
			//...
		default:
			assert(false);
		}
		break;
	}
	case WebssType::ENTITY:
		assert(webss.getEntityRaw().getContent().isAbstract());
		out += CHAR_EQUAL;
		putEntityName(out, webss.getEntityRaw());
		break;
	case WebssType::NAMESPACE:
		putNamespace(out, webss.getNamespaceRaw());
		break;
	case WebssType::ENUM:
		putEnum(out, webss.getEnumRaw());
		break;
	default:
		assert(false && "type is not an abstract value");
	}
}

void Serializer::putConcreteValue(StringBuilder& out, const Webss& webss, ConType con)
{
	switch (webss.getTypeRaw())
	{
	case WebssType::PRIMITIVE_NULL:
		out += 'N';
		break;
	case WebssType::PRIMITIVE_BOOL:
		out += webss.getBoolRaw() ? 'T' : 'F';
		break;
	case WebssType::PRIMITIVE_INT:
		putInt(out, webss.getIntRaw());
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		putDouble(out, webss.getDoubleRaw());
		break;
	case WebssType::PRIMITIVE_STRING:
		putCString(out, webss.getStringRaw());
		break;
	case WebssType::DICTIONARY:
		putDictionary(out, webss.getDictionaryRaw());
		break;
	case WebssType::LIST:
		putList(out, webss.getListRaw());
		break;
	case WebssType::LIST_TEXT:
		putListText(out, webss.getListRaw());
		break;
	case WebssType::TUPLE:
		putTuple(out, webss.getTupleRaw());
		break;
	case WebssType::TUPLE_TEXT:
		putTupleText(out, webss.getTupleRaw());
		break;
	case WebssType::TEMPLATE_BIN:
		putTemplateBin(out, webss.getTemplateBinRaw());
		break;
	case WebssType::TEMPLATE_STD:
		putTemplateStd(out, webss.getTemplateStdRaw());
		break;
	case WebssType::TEMPLATE_TEXT:
		putTemplateText(out, webss.getTemplateStdRaw());
		break;
	case WebssType::TEMPLATE_PLUS_BIN:
		putTemplatePlusBin(out, webss.getTemplatePlusBinRaw(), con);
		break;
	case WebssType::TEMPLATE_PLUS_STD:
		putTemplatePlusStd(out, webss.getTemplatePlusStdRaw(), con);
		break;
	case WebssType::TEMPLATE_PLUS_TEXT:
		putTemplatePlusText(out, webss.getTemplatePlusStdRaw(), con);
		break;
	case WebssType::ENTITY:
		assert(webss.getEntityRaw().getContent().isConcrete());
		putEntityName(out, webss.getEntityRaw());
		break;
	default:
		assert(false && "type is not a concrete value");
	}
}

void Serializer::putCharValue(StringBuilder& out, const Webss& value, ConType con)
{
	switch (value.getTypeRaw())
	{
	case WebssType::PRIMITIVE_NULL: case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT:
	case WebssType::PRIMITIVE_DOUBLE: case WebssType::ENTITY:
		out += CHAR_EQUAL;
	default:
		putConcreteValue(out, value, con);
		break;
	}
}

void Serializer::putKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	assert(value.getTypeRaw() != WebssType::DEFAULT && "can't serialize this type with key");

	out += key;
	putCharValue(out, value, con);
}

void Serializer::putExplicitKeyValue(StringBuilder& out, const string& key, const Webss& value, ConType con)
{
	out += CHAR_EXPLICIT_NAME;
	putKeyValue(out, key, value, con);
}

void Serializer::putInt(StringBuilder& out, WebssInt i)
{
	assert(i != numeric_limits<WebssInt>::min());
	out += to_string(i);
}

void Serializer::putDouble(StringBuilder& out, double d)
{
	assert(std::isfinite(d));
	char buffer[32];
	int num = snprintf(buffer, 32, "%.17g", d);
	assert(num > 0 && num < 32);
	out += buffer;
}

void addCharEscape(StringBuilder& out, char c)
{
	out += '\\';
	switch (c)
	{
	case '\0': out += '0'; break;
	case '\a': out += 'a'; break;
	case '\b': out += 'b'; break;
	case 0x1b: out += 'c'; break;
	case '\f': out += 'f'; break;
	case '\n': out += 'n'; break;
	case '\r': out += 'r'; break;
	case '\t': out += 't'; break;
	case '\v': out += 'v'; break;

	case ' ': out += 's'; break;
		//no empty char ('\e')!

	default:
		if (!isControlAscii(c))
		{
			assert(isSpecialAscii(c));
			out += c;
		}
		else
		{
			out += 'x';
			out += hexToChar(c >> 4);
			out += hexToChar(c & 0x0F);
		}
	}
}

bool isLineEnd(char c, ConType con)
{
	return c == '\n' || c == CHAR_SEPARATOR || con.isEnd(c);
}

//whether or not the char should always be escaped in a char
bool isMustEscapeChar(char c)
{
	return c == CHAR_EXPAND || c == CHAR_ESCAPE || isControlAscii(c);
}

void Serializer::putLineString(StringBuilder& out, const string& str, ConType con)
{
	if (str.empty())
		return;

	auto it = str.begin();
	//first char can't be ':'; first and last can't be ' '
	if (*it == ' ' || *it == ':')
	{
		addCharEscape(out, *it);
		if (++it == str.end())
			return;
	}

	do
	{
		if (isMustEscapeChar(*it) || isLineEnd(*it, con))
			addCharEscape(out, *it);
		else if (*it == ' ')
		{
			if (it + 1 == str.end())
			{
				out += "\\s";
				return;
			}
			else if (*(it + 1) == '/')
			{
				it += 2;
				if (it == str.end())
				{
					out += " /";
					return;
				}
				if (*it == '/' || *it == '*' || *it == '~')
					out += " \\/"; //escape junk operator
				else
					out += " /";
				out += *it;
			}
		}
		else
			out += *it;
	} while (++it != str.end());
}

void Serializer::putCString(StringBuilder& out, const string& str)
{
	out += CHAR_CSTRING;
	for (auto it = str.begin(); it != str.end(); ++it)
		if (isMustEscapeChar(*it) || *it == CHAR_CSTRING)
			addCharEscape(out, *it);
		else
			out += *it;
	out += CHAR_CSTRING;
}

void Serializer::putEntityAbstract(StringBuilder& out, const Entity& ent)
{
	out += CHAR_ABSTRACT_ENTITY;
	const auto& content = ent.getContent();
	assert(content.isAbstract());
	putEntityName(out, ent);
	putAbstractValue(out, content);
}

void Serializer::putEntityConcrete(StringBuilder& out, const Entity& ent, ConType con)
{
	out += CHAR_CONCRETE_ENTITY;
	const auto& content = ent.getContent();
	assert(content.isConcrete());
	putEntityName(out, ent);
	putCharValue(out, content, con);
}

void Serializer::putExpandDocumentHead(StringBuilder& out, const Namespace& nspace)
{
	out += CHAR_EXPAND;
	putPreviousNamespaceNames(out, nspace);
	out += nspace.getName();
}

static TheadStd makeTheadImport()
{
	TheadStd thead;
	thead.attachEmpty("name");
	thead.attach("location", ParamStd("Std"));
	thead.attach("version", ParamStd("1"));
	return thead;
}

void Serializer::putImport(StringBuilder& out, const ImportedDocument& import)
{
	static const auto thead = makeTheadImport();
	out += CHAR_IMPORT;
	const auto& data = import.getData();
	if (data.isTupleText())
		putTemplateStdTupleText(out, thead.getParams(), data.getTuple());
	else
		putTemplateStdTuple(out, thead.getParams(), data.getTuple());
}

void Serializer::putScopedImport(StringBuilder& out, const Entity& ent, const ImportedDocument& import)
{
	out += CHAR_SCOPED_IMPORT;
	putEntityName(out, ent);
	putImport(out, import);
}

void Serializer::putScopedImportList(StringBuilder& out, const vector<Entity>& entList, const ImportedDocument& import)
{
	out += CHAR_SCOPED_IMPORT;
	putSeparatedValues<vector<Entity>, ConType::LIST>(out, entList, [&](vector<Entity>::const_iterator it)
	{
		putEntityName(out, *it);
	});
	putImport(out, import);
}

void Serializer::putNamespace(StringBuilder& out, const Namespace& nspace)
{
	static const auto CON = ConType::DICTIONARY;
	NamespaceIncluder includer(currentNamespaces, nspace);
	putSeparatedValues<Namespace, CON>(out, nspace, [&](Namespace::const_iterator it)
	{
		if (it->getContent().isAbstract())
			putEntityAbstract(out, *it);
		else
			putEntityConcrete(out, *it, CON);
	});
}

void Serializer::putEnum(StringBuilder& out, const Enum& tEnum)
{
	static const auto CON = ConType::LIST;
	putSeparatedValues<Enum, CON>(out, tEnum, [&](Enum::const_iterator it) { putEntityNameWithoutNamespace(out, *it); });
}

void Serializer::putDictionary(StringBuilder& out, const Dictionary& dict)
{
	static const auto CON = ConType::DICTIONARY;
	putSeparatedValues<Dictionary, CON>(out, dict, [&](Dictionary::const_iterator it) { putKeyValue(out, it->first, it->second, CON); });
}

void Serializer::putList(StringBuilder& out, const List& list)
{
	static const auto CON = ConType::LIST;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putConcreteValue(out, *it, CON); });
}

void Serializer::putListText(StringBuilder& out, const List& list)
{
	static const auto CON = ConType::LIST;
	out += ASSIGN_CONTAINER_STRING;
	putSeparatedValues<List, CON>(out, list, [&](List::const_iterator it) { putLineString(out, it->getString(), CON); });
}

void Serializer::putTuple(StringBuilder& out, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	using Type = decltype(tuple.getOrderedKeyValues());
	putSeparatedValues<Type, CON>(out, tuple.getOrderedKeyValues(), [&](Type::const_iterator it)
	{
		if (it->first == nullptr)
			putConcreteValue(out, *it->second, CON);
		else
			putExplicitKeyValue(out, *it->first, *it->second, CON);
	});
}

void Serializer::putTupleText(StringBuilder& out, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	using Type = decltype(tuple.getOrderedKeyValues());
	out += ASSIGN_CONTAINER_STRING;
	putSeparatedValues<Type, CON>(out, tuple.getOrderedKeyValues(), [&](Type::const_iterator it)
	{
		if (it->first == nullptr)
			putLineString(out, it->second->getString(), CON);
		else
			putExplicitKeyValue(out, *it->first, *it->second, CON);
	});
}

void Serializer::putTheadSelf(StringBuilder& out)
{
	ContainerIncluder<ConType::THEAD> includer(out);
	out += CHAR_SELF;
}
void Serializer::putTheadBin(StringBuilder& out, const TheadBin& thead)
{
	static_cast<SerializerTemplate*>(this)->putTheadBin(out, thead);
}
void Serializer::putTheadStd(StringBuilder& out, const TheadStd& thead)
{
	static_cast<SerializerTemplate*>(this)->putTheadStd(out, thead);
}
void Serializer::putTheadText(StringBuilder& out, const TheadStd& thead)
{
	static_cast<SerializerTemplate*>(this)->putTheadText(out, thead);
}

void Serializer::putTemplateBin(StringBuilder& out, const TemplateBin& templ)
{
	auto putTupleRegular = [&](StringBuilder& out, const TheadBin::Params& params, const Tuple& tuple) { putTemplateBinTuple(out, params, tuple); };
	auto putTupleText = [&](StringBuilder& out, const TheadBin::Params& params, const Tuple& tuple) { putTemplateBinTuple(out, params, tuple); };

	putTheadBin(out, templ);
	const auto& params = templ.getParams();
	switch (templ.getType())
	{
	case WebssType::LIST:
		static_cast<SerializerTemplate*>(this)->putTemplateList<TheadBin::Params>(out, params, templ.getList(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		putTemplateBinTuple(out, params, templ.getTuple());
		break;
	default:
		assert(false);
	}
}

void Serializer::putTemplateStd(StringBuilder& out, const TemplateStd& templ)
{
	putTheadStd(out, templ);
	putTemplateStdBody(out, templ.getParams(), templ.getWebss());
}

void Serializer::putTemplateStdBody(StringBuilder& out, const TheadStd::Params& params, const Webss& body)
{
	auto putTupleRegular = [&](StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple) { putTemplateStdTuple(out, params, tuple); };
	auto putTupleText = [&](StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple) { putTemplateStdTupleText(out, params, tuple); };
	switch (body.getTypeRaw())
	{
	case WebssType::LIST:
		static_cast<SerializerTemplate*>(this)->putTemplateList<TheadStd::Params>(out, params, body.getListRaw(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::TUPLE:
		putTemplateStdTuple(out, params, body.getTupleRaw());
		break;
	case WebssType::TUPLE_TEXT:
		putTemplateStdTupleText(out, params, body.getTupleRaw());
		break;
	default:
		assert(false);
	}
}

void Serializer::putTemplateText(StringBuilder& out, const TemplateStd& templ)
{
	auto putTupleRegular = [&](StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple) { putTemplateTextTuple(out, params, tuple); };
	auto putTupleText = [&](StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple) { putTemplateTextTuple(out, params, tuple); };

	putTheadText(out, templ);
	const auto& params = templ.getParams();
	switch (templ.getType())
	{
	case WebssType::LIST:
		static_cast<SerializerTemplate*>(this)->putTemplateList<TheadStd::Params>(out, params, templ.getList(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::TUPLE: case WebssType::TUPLE_TEXT:
		putTemplateTextTuple(out, params, templ.getTuple());
		break;
	default:
		assert(false);
	}
}

extern void putTemplateBodyBin(StringBuilder&, const TheadBin::Params&, const Tuple&);

void Serializer::putTemplateBinTuple(StringBuilder& out, const TheadBin::Params& params, const Tuple& tuple)
{
	ContainerIncluder<ConType::TUPLE> includer(out);
	putTemplateBodyBin(out, params, tuple);
}

void Serializer::putTemplateStdTuple(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		if (param.hasThead())
			putTemplateStdBody(out, param.getTheadStd().getParams(), *it);
		else
		{
			if (it->getTypeRaw() == WebssType::NONE || it->getTypeRaw() == WebssType::DEFAULT)
				assert(param.hasDefaultValue());
			else
				putConcreteValue(out, *it, CON);
		}
	});
}

void Serializer::putTemplateStdTupleText(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	out += ASSIGN_CONTAINER_STRING;
	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		assert(!param.hasThead());
		switch (it->getTypeRaw())
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, it->getStringRaw(), CON);
			break;
		default:
			assert(false);
		}
	});
#ifdef assert
	while (i < params.size())
		assert(params[i++].hasDefaultValue());
#endif
}

void Serializer::putTemplateTextTuple(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		const auto& param = params[i++];
		switch (it->getTypeRaw())
		{
		case WebssType::NONE: case WebssType::DEFAULT:
			assert(param.hasDefaultValue());
			break;
		case WebssType::PRIMITIVE_STRING:
			putLineString(out, it->getStringRaw(), CON);
			break;
		default:
			assert(false);
		}
	});
}

void Serializer::putTemplatePlusBin(StringBuilder& out, const TemplatePlusBin& templ, ConType con)
{
	assert(templ.isTuple());
	putTheadBin(out, templ);
	putTemplateBinTuple(out, templ.getParams(), templ.getTuple());
	putConcreteValue(out, templ.getContent(), con);
}

void Serializer::putTemplatePlusStd(StringBuilder& out, const TemplatePlusStd& templ, ConType con)
{
	assert(templ.isTuple());
	putTheadStd(out, templ);
	if (templ.isTupleText())
		putTemplateStdTupleText(out, templ.getParams(), templ.getTuple());
	else
		putTemplateStdTuple(out, templ.getParams(), templ.getTuple());
	putConcreteValue(out, templ.getContent(), con);
}

void Serializer::putTemplatePlusText(StringBuilder& out, const TemplatePlusStd& templ, ConType con)
{
	assert(templ.isTuple());
	putTheadText(out, templ);
	putTemplateTextTuple(out, templ.getParams(), templ.getTuple());
	putConcreteValue(out, templ.getContent(), con);
}