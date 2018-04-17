//MIT License
//Copyright 2017 Patrick Laughrea
#include "serializer.hpp"

#include <cmath>
#include <cstdio>
#include <limits>
#include <type_traits>

#include "structures/template.hpp"
#include "structures/theadFun.hpp"
#include "utils/constants.hpp"
#include "utils/utilsWebss.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace various;
using namespace webss;

static const char* ASSIGN_CONTAINER_STRING = "::";

template <ConType::Enum CON> void putContainerStart(StringBuilder&) { assert(false); }
template <> void putContainerStart<ConType::DOCUMENT>(StringBuilder&) {}
template <> void putContainerStart<ConType::DICTIONARY>(StringBuilder& out) { out += CHAR_START_DICTIONARY; }
template <> void putContainerStart<ConType::LIST>(StringBuilder& out) { out += CHAR_START_LIST; }
template <> void putContainerStart<ConType::TUPLE>(StringBuilder& out) { out += CHAR_START_TUPLE; }
template <> void putContainerStart<ConType::THEAD>(StringBuilder& out) { out += CHAR_START_TEMPLATE; }

template <ConType::Enum CON> void putContainerEnd(StringBuilder&) { assert(false); }
template <> void putContainerEnd<ConType::DOCUMENT>(StringBuilder&) {}
template <> void putContainerEnd<ConType::DICTIONARY>(StringBuilder& out) { out += CHAR_END_DICTIONARY; }
template <> void putContainerEnd<ConType::LIST>(StringBuilder& out) { out += CHAR_END_LIST; }
template <> void putContainerEnd<ConType::TUPLE>(StringBuilder& out) { out += CHAR_END_TUPLE; }
template <> void putContainerEnd<ConType::THEAD>(StringBuilder& out) { out += CHAR_END_TEMPLATE; }

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

#ifdef NDEBUG
#define putTemplateStdTupleText(P1, P2, P3) putTemplateStdTupleText(P1, P3)
#define putTemplateTextTuple(P1, P2, P3) putTemplateTextTuple(P1, P3)
#endif

class SerializerTemplate : public Serializer
{
public:
	void putTheadBin(StringBuilder& out, const TheadBin& thead)
	{
		putThead<TheadBin, ParamBin>(out, thead, [&](StringBuilder& out, const string& key, const ParamBin& param) { putParamBin(out, key, param); });
	}

	template <class Params>
#ifdef NDEBUG
	void putTemplateList(StringBuilder& out, const Params& params, const List& list, function<void(StringBuilder& out, const Params& params, const Tuple& tuple)>&& putTupleRegular, function<void(StringBuilder& out, const Tuple& tuple)>&& putTupleText)
#else
	void putTemplateList(StringBuilder& out, const Params& params, const List& list, function<void(StringBuilder& out, const Params& params, const Tuple& tuple)>&& putTupleRegular, function<void(StringBuilder& out, const Params& params, const Tuple& tuple)>&& putTupleText)
#endif
	{
		static const auto CON = ConType::LIST;
		out += CHAR_FOREACH;
		putSeparatedValues<List, CON>(out, list, [&](typename List::const_iterator it)
		{
			assert(it->getTypeRaw() == WebssType::TEMPLATE);
			const auto& body = it->getTemplateRaw().body;
			const auto type = body.getTypeRaw();

			if (type == WebssType::TUPLE)
				putTupleRegular(out, params, body.getTupleRaw());
			else
#ifdef NDEBUG
				putTupleText(out, body.getTupleRaw());
#else
			{
				assert(type == WebssType::TUPLE_TEXT);
				putTupleText(out, params, body.getTupleRaw());
			}
#endif
		});
	}
private:
	template <class Thead, class Param>
	void putThead(StringBuilder& out, const Thead& thead, function<void(StringBuilder& out, const string& key, const Param& param)>&& putParam)
	{
		static const auto CON = ConType::THEAD;
/*		if (thead.hasEntity())
		{
			ContainerIncluder<CON> includer(out);
			out += CHAR_EXPAND;
			putEntityName(out, thead.getEntity());
		}
		else */
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
			default: assert(false);
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
			{
				ContainerIncluder<ConType::THEAD> includer(out);
				out += CHAR_SELF;
				break;
			}
			}
			putBinSizeArray(out, param.getSizeArray());
		}

		out += key;
		if (bhead.hasDefaultValue())
			putCharValue(out, bhead.getDefaultValue(), ConType::THEAD);
		else
			assert(!bhead.isTheadSelf());
	}

	void putBinSizeArray(StringBuilder& out, const ParamBin::SizeArray& barray)
	{
		using Type = ParamBin::SizeArray::Type;
		if (barray.isOne())
			return;

		ContainerIncluder<ConType::LIST> includer(out);
		switch (barray.getType())
		{
		default: assert(false);
		case Type::EMPTY:
			break;
		case Type::NUMBER:
			out += to_string(barray.size());
			break;
		case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
			putEntityName(out, barray.getEntity());
			break;
		}
	}
};

Serializer::Serializer() {}

void Serializer::putDocument(StringBuilder& out, const Document& doc)
{
	static const auto CON = ConType::DOCUMENT;
	const auto& alt = doc.getAlternate();
	if (alt.empty())
		return;
	auto itHead = doc.getHead().begin();
	auto bodyKeyValues = doc.getBody().getOrderedKeyValues();
	auto itBody = bodyKeyValues.begin();
	for (auto itAlt = alt.begin(); itAlt != alt.end(); ++itAlt)
	{
		if (*itAlt == Document::Alternate::HEAD)
		{
			assert(itHead != doc.getHead().end());
			using Type = decltype(itHead->getType());
			switch (itHead->getType())
			{
			default: assert(false);
			case Type::ENTITY_ABSTRACT:
				putEntityAbstract(out, itHead->getEntity());
				break;
			case Type::ENTITY_CONCRETE:
				putEntityConcrete(out, itHead->getEntity(), CON);
				break;
			case Type::EXPAND:
				putExpandDocumentHead(out, itHead->getNamespace());
				break;
			case Type::IMPORT:
				putImport(out, itHead->getImport(), CON);
				break;
			}
			++itHead;
			out += '\n';
		}
		else if (*itAlt == Document::Alternate::BODY)
		{
			assert(itBody != bodyKeyValues.end());
			if (itBody->first == nullptr)
				putConcreteValue(out, *itBody->second, CON);
			else
				putExplicitKeyValue(out, *itBody->first, *itBody->second, CON);
			++itBody;
			out += '\n';
		}
		else //Document::Alternate::IMPORT
			++itBody;
	}
}

void Serializer::putPreviousNamespaceNames(StringBuilder& out, const Namespace& nspace)
{
	const auto& nspaces = nspace.getNamespaces();
	if (nspaces.size() > 1) //one to exclude global namespace
	{
		//check the list of namespaces in reverse order until there is an namespace that is in
		//the current scope, then put all the namespaces that were not in the current scope

		auto i = nspaces.size();
		while (i > 1 && !namespaceCurrentScope(nspaces[i - 1])) //one to exclude global namespace
			--i;

		while (i < nspaces.size())
			out += getNameNamespaceBody(*nspaces[i++].lock()) + CHAR_SCOPE;
	}
}

bool Serializer::namespaceCurrentScope(const weak_ptr<Namespace::NamespaceBody>& nspacePtr)
{
	return currentNamespaces.find(nspacePtr.lock().get()) != currentNamespaces.end();
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
		if (!namespaceCurrentScope(nspace.getBodyPointerWeak()))
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
	case WebssType::NAMESPACE:
		putNamespace(out, webss.getNamespaceRaw());
		break;
	case WebssType::ENUM:
		putEnum(out, webss.getEnumRaw());
		break;
	case WebssType::THEAD:
		putThead(out, webss.getTheadRaw());
		break;
	case WebssType::ENTITY:
		assert(webss.getEntityRaw().getContent().isAbstract());
		out += CHAR_EQUAL;
		putEntityName(out, webss.getEntityRaw());
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
	case WebssType::STRING_LIST:
		putStringList(out, webss.getStringListRaw());
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
	case WebssType::TEMPLATE:
		putTemplate(out, webss.getTemplateRaw(), con);
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
#ifndef NDEBUG
	int num = snprintf(buffer, 32, "%.17g", d);
	assert(num > 0 && num < 32);
#else
	snprintf(buffer, 32, "%.17g", d);
#endif
	out += buffer;
}

static void addCharEscape(StringBuilder& out, char c)
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

static bool isLineEnd(char c, ConType con)
{
	return c == '\n' || c == CHAR_SEPARATOR || con.isEnd(c);
}

//whether or not the char should always be escaped in a string
static bool isMustEscapeChar(char c)
{
	return c == CHAR_SUBSTITUTION || c == CHAR_ESCAPE || isControlAscii(c);
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

static void putString(StringBuilder& out, const string& str)
{
	for (auto it = str.begin(); it != str.end(); ++it)
		if (isMustEscapeChar(*it) || *it == CHAR_CSTRING)
			addCharEscape(out, *it);
		else
			out += *it;
}

void Serializer::putCString(StringBuilder& out, const string& str)
{
	out += CHAR_CSTRING;
	putString(out, str);
	out += CHAR_CSTRING;
}

void Serializer::putStringList(StringBuilder& out, const StringList& stringList)
{
	out += CHAR_CSTRING;
	for (const auto& item : stringList.getItems())
	{
		auto type = item.getTypeRaw();
		assert(type == StringType::STRING || type == StringType::ENT_STATIC || type == StringType::WEBSS);
		if (type == StringType::STRING)
			putString(out, item.getStringRaw());
		else
		{
			out += CHAR_SUBSTITUTION;
			ContainerIncluder<ConType::DICTIONARY> includer(out);
			if (type == StringType::ENT_STATIC)
				out += item.getEntityRaw().getName();
			else
				putConcreteValue(out, item.getWebssRaw(), ConType::DICTIONARY);
		}
	}
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

void Serializer::putImport(StringBuilder& out, const ImportedDocument& import, ConType con)
{
	out += CHAR_IMPORT;
	putLineString(out, import.getLink(), con);
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

static void putTheadOptions(StringBuilder& out, TheadOptions options)
{
	//this function is called when a thead has a base (was expanded) and its options are
	//not equal to its parent's; if both options are false, then an option clear was put
	if (options.isPlus)
	{
		out += CHAR_THEAD_PLUS;
		if (options.isText)
			out += ASSIGN_CONTAINER_STRING;
	}
	else if (options.isText)
		out += ASSIGN_CONTAINER_STRING;
	else
		out += CHAR_THEAD_CLEAR;
}

void Serializer::putThead(StringBuilder& out, const Thead& thead)
{
	static const auto CON = ConType::THEAD;
	switch (thead.getTypeRaw())
	{
	default: assert(false);
	case TypeThead::BIN:
	{
		ContainerIncluder<CON> includer(out);
		const auto& theadBin = thead.getTheadBinRaw();
		if (thead.hasBase())
		{
			const auto& baseEnt = thead.getBase();
			const auto& baseThead = baseEnt.getContent().getThead();
			if (thead.isPlus() != baseThead.isPlus() || thead.isText() != baseThead.isText())
				putTheadOptions(out, thead.getOptions());
			out += CHAR_EXPAND;
			putEntityName(out, baseEnt);
			auto&& keyValues = theadBin.getParams().getOrderedKeyValues();
			for (auto i = baseThead.getTheadBin().size(); i < theadBin.size(); ++i)
			{
				out += CHAR_SEPARATOR;
				assert(keyValues[i].first != nullptr && "can't have anonymous key in dictionary, template head or enum");
				putParamBin(out, *keyValues[i].first, *keyValues[i].second);
			}
		}
		else
		{
			if (thead.isPlus() || thead.isText())
				putTheadOptions(out, thead.getOptions());
			ContainerIncluder<ConType::TEMPLATE_BIN> includerBin(out);
			for (const auto& keyValue : theadBin.getParams().getOrderedKeyValues())
			{
				assert(keyValue.first != nullptr && "can't have anonymous key in dictionary, template head or enum");
				putParamBin(out, *keyValue.first, *keyValue.second);
				out += CHAR_SEPARATOR;
			}
		}
		break;
	}
	case TypeThead::STD:
	{
		ContainerIncluder<CON> includer(out);
		const auto& theadStd = thead.getTheadStdRaw();
		if (thead.hasBase())
		{
			const auto& baseEnt = thead.getBase();
			const auto& baseThead = baseEnt.getContent().getThead();
			if (thead.isPlus() != baseThead.isPlus() || thead.isText() != baseThead.isText())
				putTheadOptions(out, thead.getOptions());
			out += CHAR_EXPAND;
			putEntityName(out, baseEnt);
			if (thead.hasModifierTuple())
				putTuple(out, thead.getModifierTuple());
			auto&& keyValues = theadStd.getParams().getOrderedKeyValues();
			for (auto i = baseThead.getTheadStd().size(); i < theadStd.size(); ++i)
			{
				out += CHAR_SEPARATOR;
				assert(keyValues[i].first != nullptr && "can't have anonymous key in dictionary, template head or enum");
				putParamStd(out, *keyValues[i].first, *keyValues[i].second);
			}
		}
		else
		{
			if (thead.isPlus() || thead.isText())
				putTheadOptions(out, thead.getOptions());
			for (const auto& keyValue : theadStd.getParams().getOrderedKeyValues())
			{
				assert(keyValue.first != nullptr && "can't have anonymous key in dictionary, template head or enum");
				putParamStd(out, *keyValue.first, *keyValue.second);
				out += CHAR_SEPARATOR;
			}
		}
		break;
	}
	case TypeThead::FUN:
		{
			ContainerIncluder<ConType::TEMPLATE_FUN> includer(out);
			for (const auto& keyValue : thead.getTheadFun().getThead().getParams().getOrderedKeyValues())
			{
				assert(keyValue.first != nullptr && "can't have anonymous key in dictionary, template head or enum");
				putParamStd(out, *keyValue.first, *keyValue.second);
				out += CHAR_SEPARATOR;
			}
		}
		//maybe template function should always have a container delimited by braces instead of
		//being free to have any value
		putConcreteValue(out, thead.getTheadFun().getStructure(), ConType::DOCUMENT);
		break;
	case TypeThead::ENTITY:
	{
		if (thead.isTheadFun())
		{
			putEntityName(out, thead.getEntityRaw());
			break;
		}
		ContainerIncluder<CON> includer(out);
		const auto& ent = thead.getEntityRaw();
		const auto& entThead = ent.getContent().getThead();
		if (thead.isPlus() != entThead.isPlus() || thead.isText() != entThead.isText())
			putTheadOptions(out, thead.getOptions());
		out += CHAR_EXPAND;
		putEntityName(out, ent);
		break;
	}
	case TypeThead::SELF:
	{
		ContainerIncluder<CON> includer(out);
		if (thead.isPlus() || thead.isText())
			putTheadOptions(out, thead.getOptions());
		out += CHAR_SELF;
		break;
	}
	}
}

void Serializer::putTheadBin(StringBuilder& out, const TheadBin& thead)
{
	static_cast<SerializerTemplate*>(this)->putTheadBin(out, thead);
}

void Serializer::putTemplate(StringBuilder& out, const Template& templ, ConType con)
{
	putThead(out, templ);

	switch (templ.getType())
	{
	default: assert(false);
	case TypeThead::BIN:
		putTemplateBinTuple(out, templ);
		break;
	case TypeThead::STD:
		if (templ.isText())
			putTemplateTextTuple(out, templ.getTheadStd().getParams(), templ.body.getTuple());
		else if (templ.body.isTupleText())
			putTemplateStdTupleText(out, templ.getTheadStd().getParams(), templ.body.getTuple());
		else
			putTemplateStdTuple(out, templ.getTheadStd().getParams(), templ.body.getTuple());
		break;
	case TypeThead::FUN: //theadFun body is serialized exactly like theadStd body
		if (templ.isText())
			putTemplateTextTuple(out, templ.getTheadFun().getThead().getParams(), templ.body.getTuple());
		else if (templ.body.isTupleText())
			putTemplateStdTupleText(out, templ.getTheadFun().getThead().getParams(), templ.body.getTuple());
		else
			putTemplateStdTuple(out, templ.getTheadFun().getThead().getParams(), templ.body.getTuple());
		break;
	}

	if (templ.isPlus())
		putConcreteValue(out, templ.content, con);
}

extern void putTemplateBodyBin(StringBuilder&, const TheadBin::Params&, const Tuple&, bool);

void Serializer::putTemplateBinTuple(StringBuilder& out, const Template& templ)
{
	ContainerIncluder<ConType::TUPLE> includer(out);
	putTemplateBodyBin(out, templ.getTheadBin().getParams(), templ.body.getTuple(), templ.isText());
}

void Serializer::putTemplateStdBody(StringBuilder& out, const TheadStd::Params& params, const Webss& body)
{
	auto putTupleRegular = [&](StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple) { putTemplateStdTuple(out, params, tuple); };
#ifdef NDEBUG
	auto putTupleText = [&](StringBuilder& out, const Tuple& tuple) { putTemplateStdTupleText(out, dummy, tuple); };
#else
	auto putTupleText = [&](StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple) { putTemplateStdTupleText(out, params, tuple); };
#endif
	switch (body.getTypeRaw())
	{
	default: assert(false);
	case WebssType::LIST:
		static_cast<SerializerTemplate*>(this)->putTemplateList<TheadStd::Params>(out, params, body.getListRaw(), move(putTupleRegular), move(putTupleText));
		break;
	case WebssType::TUPLE:
		putTemplateStdTuple(out, params, body.getTupleRaw());
		break;
	case WebssType::TUPLE_TEXT:
		putTemplateStdTupleText(out, params, body.getTupleRaw());
		break;
	}
}

void Serializer::putTemplateStdTuple(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	decltype(params.size()) i = 0;
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		assert(it->getTypeRaw() != WebssType::NONE);
		const auto& param = params[i++];
		if (param.hasThead())
			putTemplateStdBody(out, param.getTheadStd().getParams(), *it);
		else if (it->getTypeRaw() != WebssType::DEFAULT)
			putConcreteValue(out, *it, CON);
		else
			assert(param.hasDefaultValue());
	});
}

void Serializer::putTemplateStdTupleText(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");

	out += ASSIGN_CONTAINER_STRING;
#ifndef NDEBUG
	decltype(params.size()) i = 0;
#endif
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
#ifndef NDEBUG
		const auto& param = params[i++];
		assert(!param.hasThead());
#endif
		if (it->getTypeRaw() == WebssType::PRIMITIVE_STRING)
			putLineString(out, it->getStringRaw(), CON);
#ifndef NDEBUG
		else if (it->getTypeRaw() == WebssType::DEFAULT)
			assert(param.hasDefaultValue());
		else
			assert(false);
#endif
	});
#ifndef NDEBUG
	while (i < params.size())
		assert(params[i++].hasDefaultValue());
#endif
}

void Serializer::putTemplateTextTuple(StringBuilder& out, const TheadStd::Params& params, const Tuple& tuple)
{
	static const auto CON = ConType::TUPLE;
	assert(tuple.size() <= params.size() && "too many elements in template tuple");
	
#ifndef NDEBUG
	decltype(params.size()) i = 0;
#endif
	putSeparatedValues<Tuple, CON>(out, tuple, [&](Tuple::const_iterator it)
	{
		if (it->getTypeRaw() == WebssType::PRIMITIVE_STRING)
			putLineString(out, it->getStringRaw(), CON);
#ifndef NDEBUG
		else if (it->getTypeRaw() == WebssType::DEFAULT)
			assert(params[i++].hasDefaultValue());
		else
			assert(false);
#endif
	});
}

void Serializer::putParamBin(StringBuilder& out, const string& key, const ParamBin& param)
{
	const auto& bhead = param.getSizeHead();
	using Type = ParamBin::SizeHead::Type;
	switch (bhead.getType())
	{
	default: assert(false);
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
	{
		ContainerIncluder<ConType::THEAD> includer(out);
		out += CHAR_SELF;
		break;
	}
	}
	putBinSizeArray(out, param.getSizeArray());
	out += CHAR_TEMPLATE_BIN_SEPARATOR;
	out += key;
	if (bhead.hasDefaultValue())
		putCharValue(out, bhead.getDefaultValue(), ConType::THEAD);
	else
		assert(!bhead.isTheadSelf());
}

void Serializer::putBinSizeArray(StringBuilder& out, const ParamBin::SizeArray& barray)
{
	using Type = ParamBin::SizeArray::Type;
	if (barray.isOne())
		return;

	ContainerIncluder<ConType::TEMPLATE_BIN_ARRAY> includer(out);
	switch (barray.getType())
	{
	default: assert(false);
	case Type::EMPTY:
		break;
	case Type::NUMBER:
		out += to_string(barray.size());
		break;
	case Type::EMPTY_ENTITY_NUMBER: case Type::ENTITY_NUMBER:
		putEntityName(out, barray.getEntity());
		break;
	}
}

void Serializer::putParamStd(StringBuilder& out, const string& key, const ParamStd& param)
{
	if (param.hasThead())
		putThead(out, param.getThead());

	out += key;
	if (param.hasDefaultValue())
		putCharValue(out, param.getDefaultValue(), ConType::THEAD);
	else
		assert(!param.hasThead() || param.getTypeThead() != TypeThead::SELF);
}