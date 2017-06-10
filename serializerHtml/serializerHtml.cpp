//MIT License
//Copyright 2017 Patrick Laughrea
#include "serializerHtml.hpp"

#include "structures/paramStandard.hpp"
#include "structures/tuple.hpp"
#include "utils/utils.hpp"

using namespace std;
using namespace webss;

SerializerHtml::SerializerHtml() {}

void SerializerHtml::putDocument(StringBuilder& out, const Document& doc)
{
	out += "<!DOCTYPE html>";
	out += "<html>";
	for (const auto& item : doc.getData())
	{
		assert(item.isTemplateStandard());
		putTemplStandard(out, item.getTemplateStandard());
	}
	out += "</html>";
}

void SerializerHtml::putTemplStandard(StringBuilder& out, const TemplateStandard& templ)
{
	assert(templ.hasEntity() && templ.isTuple());
	const auto& name = templ.getEntity().getName();
	const auto& tuple = templ.getTuple();
	const auto& params = templ.getParameters();
	assert(tuple.size() == params.size() || tuple.size() == params.size() + 1);
	bool isBlock = tuple.size() > params.size();
	auto keyValues = tuple.getOrderedKeyValues();
	out += "<" + name;
	for (Tuple::size_type i = 0; i < tuple.size() - (isBlock ? 1 : 0); ++i)
	{
		assert(keyValues[i].first != nullptr);
		const auto& key = *keyValues[i].first;
		const auto& value = *keyValues[i].second;
		out += ' ' + key + "=\"";
		if (value.getTypeRaw() == WebssType::NONE || value.getTypeRaw() == WebssType::DEFAULT)
			putConcreteValue(out, params[i].getDefaultValue());
		else
			putConcreteValue(out, value);
		out += '"';
	}

	if (!isBlock)
		out += " />";
	else
	{
		out += '>';
		putConcreteValue(out, tuple.back());
		out += "</" + name + '>';
	}
}

void SerializerHtml::putConcreteValue(StringBuilder& out, const Webss& webss)
{
	switch (webss.getTypeRaw())
	{
	case WebssType::PRIMITIVE_BOOL:
		out += webss.getBoolRaw() ? "true" : "false";
		break;
	case WebssType::PRIMITIVE_INT:
		putInt(out, webss.getIntRaw());
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		putDouble(out, webss.getDoubleRaw());
		break;
	case WebssType::PRIMITIVE_STRING:
		putString(out, webss.getStringRaw());
		break;
	case WebssType::ENTITY:
		assert(webss.getEntityRaw().getContent().isConcrete());
		putConcreteValue(out, webss.getEntityRaw().getContent());
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

bool isMustEscapeChar(char c)
{
	return c == '&' || c == '<' || c == '>' || c == '"' || c == '\'' || isControlAscii(c);
}

void addCharEscape(StringBuilder& out, char c)
{
	out += '&';
	switch (c)
	{
	case '&': out += "amp"; break;
	case '<': out += "lt"; break;
	case '>': out += "gt"; break;
	case '"': out += "quot"; break;
	case '\'': out += "apos"; break;
	default:
		assert(isControlAscii(c));
		out += 'x';
		out += hexToChar(c >> 4);
		out += hexToChar(c & 0x0F);
	}
	out += ';';
}

void SerializerHtml::putString(StringBuilder& out, const string& str)
{
	for (auto it = str.begin(); it != str.end(); ++it)
		if (isMustEscapeChar(*it))
			addCharEscape(out, *it);
		else if (*it == ' ')
		{
			out += ' ';
			if (++it == str.end())
				break;
			if (*it == ' ')
				out += "&nbsp;";
			else
				out += *it;
		}
		else
			out += *it;
}