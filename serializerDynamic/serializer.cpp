//All rights reserved
//Copyright 2018 Patrick Laughrea
#include "serializer.hpp"

#include <cmath>
#include <iostream>
#include <set>

#include "structures/list.hpp"
#include "structures/paramStandard.hpp"
#include "structures/placeholder.hpp"
#include "structures/template.hpp"
#include "structures/theadFun.hpp"
#include "structures/tuple.hpp"
#include "structures/webss.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace webss;
using namespace various;

SerializerDynamic::SerializerDynamic() {}

void SerializerDynamic::putDocument(const Document& doc)
{
	//the dynamic serializer probably needs to be more involved with the parser
	for (const auto& item : doc.getBody())
		putConcreteValue(item);
}

//a concrete value in this context is a command
void SerializerDynamic::putConcreteValue(const Webss& value)
{
	switch (value.getTypeRaw())
	{
	case WebssType::PRINT_STRING:
		cout << value.getStringRaw();
		break;
	case WebssType::PRINT_STRING_LIST:
		for (const auto& item : value.getStringListRaw().getItems())
		{
			//for strings within quotes, make sure all quotes and '\\', etc., are escaped properly
			StringType type = item.getTypeRaw();
			switch (type)
			{
			case StringType::STRING:
				cout << item.getStringRaw();
				break;
			case StringType::ENT_STATIC:
				cout << item.getEntityRaw().getContent().getString();
				break;
			case StringType::FUNC_NEWLINE:
				cout.put(cout.widen('\n'));
				break;
			case StringType::FUNC_FLUSH:
				cout.flush();
				break;
			case StringType::FUNC_NEWLINE_FLUSH:
				cout << endl;
				break;
			//case StringType::WEBSS:
			//serialize to string... that is, WebSSON
			//	out += "std::out << \"" + item.getWebssRaw() + ";";
			case StringType::ENT_DYNAMIC:
				throw runtime_error("cannot print dynamic entity"); //runtime_error since parser does not check for this
			default:
				assert(false);
			}
		}
		break;
/*
	case WebssType::PRIMITIVE_BOOL: case WebssType::PRIMITIVE_INT: case WebssType::PRIMITIVE_DOUBLE:
		putQuotableValue(out, value);
		break;
	case WebssType::PRIMITIVE_STRING:
		putDocumentString(out, value.getStringRaw());
		break;
	case WebssType::ENTITY:
		assert(value.getEntityRaw().getContent().isConcrete());
		putConcreteValue(out, value.getEntityRaw().getContent());
		break;
	case WebssType::TEMPLATE:
		putTemplate(out, value.getTemplateRaw());
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT:
		putList(out, value.getListRaw());
		break;
	case WebssType::PLACEHOLDER:
		putConcreteValue(out, value.getPlaceholderRaw().getValue());
		break;
*/
	default:
		assert(false && "type is not a concrete value or is not serializable in C++");
	}
}

/*
void SerializerCppHtml::putQuotableValue(StringBuilder& out, const Webss& value)
{
	switch (value.getTypeRaw())
	{
	case WebssType::PRIMITIVE_BOOL:
		out += value.getBoolRaw() ? "true" : "false";
		break;
	case WebssType::PRIMITIVE_INT:
		putInt(out, value.getIntRaw());
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		putDouble(out, value.getDoubleRaw());
		break;
	case WebssType::PRIMITIVE_STRING:
		putQuotableString(out, value.getStringRaw());
		break;
	case WebssType::ENTITY:
		assert(value.getEntityRaw().getContent().isConcrete());
		putQuotableValue(out, value.getEntityRaw().getContent());
		break;
	default:
		assert(false && "type is not a quotable value");
	}
}

void SerializerCppHtml::putKeyValue(StringBuilder& out, const string& key, const Webss& value)
{
	if (value.isBool())
	{
		if (equalAny(key, { "async", "autofocus", "autoplay", "checked", "controls",
				"default", "defer", "disabled", "download", "formnovalidate", "hidden", "ismap", "loop",
				"multiple", "muted", "novalidate", "open", "readonly", "required",
				"reversed", "sandbox", "scoped", "selected" }))
		{
			if (value.getBool())
				out += ' ' + key;
			return;
		}
		else if (key == "autocomplete")
		{
			out += ' ' + key + "=\"" + (value.getBool() ? "on" : "off") + '"';
			return;
		}
		else if (key == "translate")
		{
			out += ' ' + key + "=\"" + (value.getBool() ? "yes" : "no") + '"';
			return;
		}
		else if (equalAny(key, { "contenteditable", "draggable", "spellcheck" }))
			{
				//skip to print key-value with false and true
			}
	}

	out += ' ' + key + "=\"";
	putQuotableValue(out, value);
	out += '"';
}

void SerializerCppHtml::putInt(StringBuilder& out, WebssInt i)
{
	assert(i != numeric_limits<WebssInt>::min());
	out += to_string(i);
}

void SerializerCppHtml::putDouble(StringBuilder& out, double d)
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

static bool isMustEscapeChar(char c)
{
	return c == '"' || c == '\'' || isControlAscii(c);
}

static void addCharEscape(StringBuilder& out, char c)
{
	out += '&';
	out += 'x';
	out += hexToChar(c >> 4);
	out += hexToChar(c & 0x0f);
	out += ';';
}

void SerializerCppHtml::putQuotableString(StringBuilder& out, const string& str)
{
	for (auto it = str.begin(); it != str.end(); ++it)
		if (isMustEscapeChar(*it))
			addCharEscape(out, *it);
		else
			out += *it;
}

void SerializerCppHtml::putDocumentString(StringBuilder& out, const string& str)
{
	out += str;
}

static bool isDefaultValue(const Webss& value)
{
	return value.getTypeRaw() == WebssType::DEFAULT;
}

static const string& getTheadRootName(const Thead& thead)
{
	const Thead* ptrThead = &thead;
	const string* name = nullptr;
	while (true)
	{
		const Entity* ent;
		if (ptrThead->hasEntity())
			ent = &ptrThead->getEntityRaw();
		else if (ptrThead->hasBase())
			ent = &ptrThead->getBase();
		else
			break;

		name = &ent->getName();
		ptrThead = &ent->getContent().getThead();
	}
	assert(name != nullptr);
	return *name;
}

void SerializerCppHtml::putTemplate(StringBuilder& out, const Template& templ)
{
	if (templ.getType() == TypeThead::FUN)
	{
		putTemplateFun(out, templ);
		return;
	}
	assert(templ.body.isTuple() && templ.getType() == TypeThead::STD);
	const auto& name = getTheadRootName(templ);
	const auto& tuple = templ.body.getTuple();
	const auto& params = templ.getTheadStd().getParams();
	auto keys = tuple.getOrderedKeys();
	out += "<" + name;
	for (Tuple::size_type i = 0; i < tuple.size(); ++i)
	{
		assert(keys[i] != nullptr);
		const auto& key = *keys[i];
		const auto& value = tuple[i];
		const auto& param = params[i];
		if (value.isNull())
			assert(isDefaultValue(value));
		else
			putKeyValue(out, key, isDefaultValue(value) ? param.getDefaultValue() : value);
	}
	out += '>';
	if (templ.isPlus())
	{
		putConcreteValue(out, templ.content);
		out += "</" + name + '>';
	}
}

void SerializerCppHtml::putList(StringBuilder& out, const List& list)
{
	for (const auto& item : list)
		putConcreteValue(out, item);
}

void SerializerCppHtml::putTemplateFun(StringBuilder& out, const Template& templ)
{
	const auto& theadFun = templ.getTheadFun();
	const auto& tuple = templ.body.getTuple();
	TheadFunPointer ptr = theadFun.setPointer(&tuple);
	putConcreteValue(out, theadFun.getStructure());
}
*/