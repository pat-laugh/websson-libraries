//All rights reserved
//Copyright 2018 Patrick Laughrea
#include "serializer.hpp"

#include <cmath>
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

SerializerCpp::SerializerCpp() {}

void putEndCmd(StringBuilder& out)
{
	out += ';';
	out += '\n';
}

void SerializerCpp::putDocument(StringBuilder& out, const Document& doc)
{
	out += "#include <iostream>\n"; //newline should be replace by char.widen or whatever
									//make it so StringBuilder has something like putNewline
	
	//first pass through all the abstract values to put all the function signatures, etc.
	//doc.getHead()...
	
	out += "int main(int argc, char** argv) {\n"; //style will eventually be customizable
												//but what takes the fewest lines is easier for now
	for (const auto& item : doc.getBody())
		putConcreteValue(out, item);
	out += "return 0";
	putEndCmd(out);
	out += "}";
	
	//other notes: it could be noted that {...} are a concept of block... other
	//languages have a different notation for that
	//same with ';', which ends commands. Other languages use different stuff
}

static void checkCharEscape(StringBuilder& out, char c)
{
	switch (c)
	{
	//don't put null char as \0 because in C and C++ \0 is actually an octal escape that takes 1 to 3 digits -- you'd have to put \000 to make sure it does not get messed up
	case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06:
	/*0x07 to 0x0d are the letters*/ case 0x0e: case 0x0f:
	case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
	case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
	case 0x7f:
		out += '\\';
		out += 'x';
		out += hexToChar(c >> 4);
		out += hexToChar(c & 0x0F);
		break;
	case '\'': c = '\''; goto specialEscape;
	case '\"': c = '\"'; goto specialEscape;
	case '\?': c = '\?'; goto specialEscape; //avoid trigraphs
	case '\\': c = '\\'; goto specialEscape;
	case '\a': c = 'a'; goto specialEscape;
	case '\b': c = 'b'; goto specialEscape;
	case '\f': c = 'f'; goto specialEscape;
	case '\n': c = 'n'; goto specialEscape;
	case '\r': c = 'r'; goto specialEscape;
	case '\t': c = 't'; goto specialEscape;
	case '\v': c = 'v'; goto specialEscape;
specialEscape:
		out += '\\';
	default:
		out += c;
		break;
	}
}

static void putString(StringBuilder& out, const string& str)
{
	out += '"';
	for (char c : str)
		checkCharEscape(out, c);
	out += '"';
}

//a concrete value in this context is a command or a value
void SerializerCpp::putConcreteValue(StringBuilder& out, const Webss& value)
{
	switch (value.getTypeRaw())
	{
	case WebssType::PRINT_STRING:
		out += "std::cout << ";
		putString(out, value.getStringRaw());
		putEndCmd(out);
		break;
	case WebssType::PRINT_STRING_LIST:
		for (const auto& item : value.getStringListRaw().getItems())
		{
			//for strings within quotes, make sure all quotes and '\\', etc., are escaped properly
			StringType type = item.getTypeRaw();
			switch (type)
			{
			case StringType::STRING:
				out += "std::cout << ";
				putString(out, item.getStringRaw());
				putEndCmd(out);
				break;
			case StringType::ENT_STATIC:
				out += "std::cout << ";
				out += item.getEntityRaw().getName();
				putEndCmd(out);
				break;
			case StringType::FUNC_NEWLINE:
				out += "std::cout.put(std::cout.widen('\\n'))";
				putEndCmd(out);
				break;
			case StringType::FUNC_FLUSH:
			 	out += "std::cout.flush()";
				putEndCmd(out);
				break;
			case StringType::FUNC_NEWLINE_FLUSH:
				out += "std::cout << std::endl";
				putEndCmd(out);
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