//MIT License
//Copyright 2017 Patrick Laughrea
#include "serializerHtml.hpp"

#include <cmath>
#include <set>

#include "structures/paramStandard.hpp"
#include "structures/templatePlus.hpp"
#include "structures/tuple.hpp"
#include "utils/utils.hpp"

using namespace std;
using namespace webss;

bool equalAny(const string& name, set<string> names)
{
	return names.find(name) != names.end();
}

SerializerHtml::SerializerHtml() {}

void SerializerHtml::putDocument(StringBuilder& out, const Document& doc)
{
	out += "<!DOCTYPE html>";
	out += "<html>";
	for (const auto& item : doc.getData())
		putConcreteValue(out, item);
	out += "</html>";
}

void SerializerHtml::putQuotableValue(StringBuilder& out, const Webss& value)
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

void SerializerHtml::putKeyValue(StringBuilder& out, const string& key, const Webss& value)
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
			{ /* skip to print key-value with false and true */ }
	}

	out += ' ' + key + "=\"";
	putQuotableValue(out, value);
	out += '"';
}

void SerializerHtml::putConcreteValue(StringBuilder& out, const Webss& value)
{
	switch (value.getTypeRaw())
	{
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
	case WebssType::TEMPLATE_STANDARD: case WebssType::TEMPLATE_TEXT:
		putTemplateStandard(out, value.getTemplateStandardRaw());
		break;
	case WebssType::TEMPLATE_PLUS_STANDARD: case WebssType::TEMPLATE_PLUS_TEXT:
		putTemplatePlusStandard(out, value.getTemplatePlusStandardRaw());
		break;
	case WebssType::LIST: case WebssType::LIST_TEXT:
		putList(out, value.getListRaw());
		break;
	default:
		assert(false && "type is not a concrete value");
	}
}

void SerializerHtml::putInt(StringBuilder& out, WebssInt i)
{
	assert(i != numeric_limits<WebssInt>::min());
	out += to_string(i);
}

void SerializerHtml::putDouble(StringBuilder& out, double d)
{
	assert(std::isfinite(d));
	char buffer[32];
	int num = snprintf(buffer, 32, "%.17g", d);
	assert(num > 0 && num < 32);
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

void SerializerHtml::putQuotableString(StringBuilder& out, const string& str)
{
	for (auto it = str.begin(); it != str.end(); ++it)
		if (isMustEscapeChar(*it))
			addCharEscape(out, *it);
		else
			out += *it;
}

void SerializerHtml::putDocumentString(StringBuilder& out, const string& str)
{
	out += str;
}

bool isDefaultValue(const Webss& value)
{
	return value.getTypeRaw() == WebssType::NONE || value.getTypeRaw() == WebssType::DEFAULT;
}

void SerializerHtml::putTemplateStandard(StringBuilder& out, const TemplateStandard& templ)
{
	assert(templ.hasEntity() && templ.isTuple());
	const auto& name = templ.getEntity().getName();
	const auto& tuple = templ.getTuple();
	const auto& params = templ.getParameters();
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
}

void SerializerHtml::putTemplatePlusStandard(StringBuilder& out, const TemplatePlusStandard& templ)
{
	putTemplateStandard(out, templ);
	putConcreteValue(out, templ.getContent());
	out += "</" + templ.getEntity().getName() + '>';
}

void SerializerHtml::putList(StringBuilder& out, const List& list)
{
	for (const auto& item : list)
		putConcreteValue(out, item);
}
