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
#include "utils/constants.hpp"
#include "various/utils.hpp"

using namespace std;
using namespace webss;
using namespace various;

static bool typeIsString(WebssType type)
{
	return type == WebssType::PRIMITIVE_STRING || type == WebssType::STRING_LIST;
}

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
	out += "#include <string>\n";
	
	//first pass through all the abstract values to put all the function signatures, etc.
	//doc.getHead()...
	
	for (const auto& item : doc.getHead())
	{
		using Type = decltype(item.getType());
		switch (item.getType())
		{
		default: assert(false);
		/*
		case Type::ENTITY_ABSTRACT:
			putEntityAbstract(out, item.getEntity());
			break;
		*/
		case Type::ENTITY_CONCRETE:
			putEntityConcrete(out, item.getEntity());
			break;
		/*	
		case Type::EXPAND:
			putExpandDocumentHead(out, item.getNamespace());
			break;
		case Type::IMPORT:
			putImport(out, item.getImport(), CON);
			break;
		*/
		}
	}
	
	out += "int main(int argc, char** argv) {\n"; //style will eventually be customizable
												//but what takes the fewest lines is easier for now
	for (const auto& item : doc.getBody())
		putCommand(out, item);
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

static void putCString(StringBuilder& out, const string& str)
{
	out += '"';
	for (char c : str)
		checkCharEscape(out, c);
	out += '"';
}

static void putString(StringBuilder& out, const string& str)
{
	out += "std::string(";
	putCString(out, str);
	out += ")";
}

void SerializerCpp::putStringList(StringBuilder& out, const StringList& slist)
{
	out += "std::string()"; //so that everything adds as string
	for (const auto& item : slist.getItems())
	{
		StringType type = item.getTypeRaw();
		switch (type)
		{
		case StringType::STRING:
			out += " + ";
			putString(out, item.getStringRaw());
			break;
		case StringType::ENT_STATIC:
			out += " + ";
			putEntityName(out, item.getEntityRaw());
			break;
		case StringType::FUNC_NEWLINE:
		case StringType::FUNC_FLUSH:
		case StringType::FUNC_NEWLINE_FLUSH:
			throw runtime_error("cannot put string function"); //runtime_error since parser does not check for this
		//case StringType::WEBSS:
		//serialize to string... that is, WebSSON
		//	out += "std::out << \"" + item.getWebssRaw() + ";";
		case StringType::ENT_DYNAMIC:
		//	throw runtime_error("cannot print dynamic entity"); //runtime_error since parser does not check for this
		default:
			assert(false);
		}
	}
}

void SerializerCpp::putCommand(StringBuilder& out, const Webss& webss)
{
	switch (webss.getTypeRaw())
	{
	case WebssType::PRIMITIVE_STRING:
		out += "std::cout << ";
		putString(out, webss.getStringRaw());
		putEndCmd(out);
		break;
	case WebssType::STRING_LIST:
	{
		bool putStart = true;
		for (const auto& item : webss.getStringListRaw().getItems())
		{
			StringType type = item.getTypeRaw();
			switch (type)
			{
			case StringType::STRING:
				if (putStart)
				{
					out += "std::cout";
					putStart = false;
				}
				out += " << ";
				putString(out, item.getStringRaw());
				break;
			case StringType::ENT_STATIC:
				if (putStart)
				{
					out += "std::cout";
					putStart = false;
				}
				out += " << ";
				putEntityName(out, item.getEntityRaw());
				break;
			case StringType::FUNC_NEWLINE:
				if (!putStart)
					putEndCmd(out);
				out += "std::cout.put(std::cout.widen('\\n'))";
				putEndCmd(out);
				putStart = true;
				break;
			case StringType::FUNC_FLUSH:
				if (!putStart)
					putEndCmd(out);
			 	out += "std::cout.flush()";
				putEndCmd(out);
				putStart = true;
				break;
			case StringType::FUNC_NEWLINE_FLUSH:
				if (putStart)
				{
					out += "std::cout << ";
					putStart = false;
				}
				out += "std::endl";
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
		putEndCmd(out);
	}
		break;
	case WebssType::LIST:
		out += "{\n";
		for (const auto& item : webss.getListRaw())
			putCommand(out, item);
		out += "}\n";
		break;
	case WebssType::FOREACH:
	{
		const auto& webssForeach = webss.getElementRaw<Webss>();
		if (webssForeach.isTemplate())
		{
			const auto& templ = webssForeach.getTemplate();
			const auto& cont = templ.body.getTuple()[0];
			auto type = cont.getType();
			string nameParam = "item0";
			if (typeIsString(type))
				out += "for (char " + nameParam + " : ";
			else
				out += "for (const auto& " + nameParam + " : ";
			putConcreteValue(out, cont);
			out += ") ";
			nameSubst.insert({"$_", nameParam});
			if (templ.content.getTypeRaw() == WebssType::LIST)
				putCommand(out, templ.content);
			else
			{
				out += "{";
				putCommand(out, templ.content);
				out += "}\n";
			}
			nameSubst.erase("$_");
		}
	}
		break;
	default:
		assert(false && "type is not a command serializable in C++");
	}
}

//a concrete webss in this context is a command or a webss
void SerializerCpp::putConcreteValue(StringBuilder& out, const Webss& webss)
{
	switch (webss.getTypeRaw())
	{
	case WebssType::PRIMITIVE_BOOL:
		out += to_string(webss.getBool());
		break;
	case WebssType::PRIMITIVE_INT:
		out += to_string(webss.getInt());
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		out += to_string(webss.getDouble());
		break;
	case WebssType::PRIMITIVE_STRING:
		putString(out, webss.getStringRaw());
		break;
	case WebssType::STRING_LIST:
		putStringList(out, webss.getStringList());
		break;
	case WebssType::ENTITY:
		assert(webss.getEntityRaw().getContent().isConcrete());
		putEntityName(out, webss.getEntityRaw());
		break;
/*	case WebssType::LIST:
		putList(out, webss.getListRaw());
		break;
	case WebssType::FOREACH:
		//put a vector that contains a list as a return of this
		break;*/
	default:
		assert(false && "type is not a concrete webss or is not serializable in C++");
	}
}

void SerializerCpp::putConcreteType(StringBuilder& out, Webss webss)
{
	switch (webss.getType())
	{
	case WebssType::PRIMITIVE_BOOL:
		out += "bool";
		break;
	case WebssType::PRIMITIVE_INT:
		out += "int";
		break;
	case WebssType::PRIMITIVE_DOUBLE:
		out += "double";
		break;
	case WebssType::PRIMITIVE_STRING:
		out += "std::string";
		break;
	case WebssType::STRING_LIST:
		out += "std::string";
		break;
	default:
		assert(false);
	}
}

void SerializerCpp::putEntityName(StringBuilder& out, const Entity& ent)
{
	auto it = nameSubst.find(ent.getName());
	if (it == nameSubst.end())
		out += ent.getName();
	else
		out += it->second;
/*	if (ent.hasNamespace())
	{
		const auto& nspace = ent.getNamespace();
		if (!namespaceCurrentScope(nspace.getBodyPointerWeak()))
		{
			putPreviousNamespaceNames(out, nspace);
			out += nspace.getName() + CHAR_SCOPE;
		}
	}*/
}

void SerializerCpp::putEntityConcrete(StringBuilder& out, const Entity& ent)
{
	const auto& content = ent.getContent();
	assert(content.isConcrete());
	putConcreteType(out, content);
	out += " ";
	putEntityName(out, ent);
	out += " = ";
	putConcreteValue(out, ent.getContent().getWebssLast());
	putEndCmd(out);
}

void SerializerCpp::putList(StringBuilder& out, const List& list)
{
	//assume for now list contains only integers
}