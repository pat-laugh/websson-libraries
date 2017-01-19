//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "railroadDiagrams.h"

using namespace std;
using namespace webss;

string Deserializer::deserializeStuff(const Webss& webss)
{
	switch (webss.getType())
	{
	case WebssType::PRIMITIVE_NULL:
		return "";
	case WebssType::PRIMITIVE_STRING:
		return deserializeString(webss.getString());
	case WebssType::TUPLE:
		return deserializeTuple(webss.getTuple());
//	case WebssType::BLOCK:
//		return deserializeBlock(webss.getBlock());
	case WebssType::LIST:
		return deserializeList(webss.getList());
	default:
		throw runtime_error("invalid data type: " + webss.t.toString());
	}
}

string cleanString(const string& s)
{
	string out;
	out.reserve(s.size());
	for (char c : s)
		switch (c)
		{
		case '\0': out += "\\\\0"; break;
		case '\a': out += "\\\\a"; break;
		case '\b': out += "\\\\b"; break;
		case '\f': out += "\\\\f"; break;
		case '\n': out += "\\\\n"; break;
		case '\r': out += "\\\\r"; break;
		case '\t': out += "\\\\t"; break;
		case '\v': out += "\\\\v"; break;
		case '\'': out += "\\\'"; break;
		case '\"': out += "\\\""; break;
		case '\\': out += "\\\\"; break;

		default:
			if (!isControlAscii(c))
				out += c;
			else
			{
				out += "\\\\x";
				out += hexToChar(c >> 4);
				out += hexToChar(c & 0x0F);
			}
		}
	return out;
}

string Deserializer::deserializeString(const string& s)
{
	return "Terminal('" + cleanString(s) + "')";
}

string Deserializer::deserializeTuple(const Tuple& tuple)
{
	if (tuple.empty())
		throw runtime_error("can't have empty tuple");
	else if (tuple.size() == 1)
		return "NonTerminal('" + cleanString(tuple[0].getString()) + "')";
	else
		return "NonTerminal('" + cleanString(tuple[0].getString()) + "','" + cleanString(tuple[1].getString()) + "')";
}

string Deserializer::deserializeOptional(const Webss& value)
{
	string out = deserializeStuff(value);
	return out.empty() ? "" : "Optional(" + out + ')';
}

string Deserializer::deserializeLoopParam(const Webss& value)
{
	switch (value.getType())
	{
	case WebssType::PRIMITIVE_NULL:
		return "";
	case WebssType::PRIMITIVE_STRING:
		return ',' + deserializeString(value.getString());
	case WebssType::TUPLE:
		return ',' + deserializeTuple(value.getTuple());
	case WebssType::LIST: //output in reverse
	{
		const auto& list = value.getList();
		auto it = list.rbegin();
		return ",Sequence(" + getSeparatedValues([&]()
		{
			return ++it != list.rend();
		}, [&]()
		{
			return deserializeStuff(*it);
		}) + ')';
	}
	default:
		throw runtime_error("invalid loop param data type: " + value.getType().toString());
	}
}

const char ERROR_LOOP[] = "loop must be a list of at least two elements";
string Deserializer::deserializeLoop(const Webss& value)
{
	if (!value.isList())
		throw runtime_error(ERROR_LOOP);

	string out = "OneOrMore(";
	string temp;
	const auto& list = value.getList();
	if (list.size() > 2)
	{
		auto it = list.begin() + 1;
		out = getSeparatedValues([&]()
		{
			return ++it != list.end();
		}, [&]()
		{
			return deserializeStuff(*it);
		});
		if (out.empty())
		{
			out = deserializeStuff(list[0]);
			if (out.empty())
				return "";
			else
				return "ZeroOrMore(" + out + ')';
		}
		out = "OneOrMore(Sequence(" + out + ')';
	}
	else if (list.size() == 2)
	{
		out = deserializeStuff(list[1]);
		if (out.empty())
		{
			out = deserializeStuff(list[0]);
			if (out.empty())
				return "";
			else
				return "ZeroOrMore(" + out + ')';
		}
		out = "OneOrMore(" + out;
	}
	else
		throw runtime_error(ERROR_LOOP);

	return out + deserializeLoopParam(list[0]) + ')';
}

const char ERROR_SWITCH[] = "switch must be a list of at least two elements";
string Deserializer::deserializeSwitch(const Webss& value)
{
	if (!value.isList())
		throw runtime_error(ERROR_SWITCH);

	const auto& list = value.getList();
	if (list.size() >= 2)
	{
		auto it = list.begin();
		string out = getSeparatedValues([&]()
		{
			return ++it != list.end();
		}, [&]()
		{
			return deserializeStuff(*it);
		});
		if (out.empty())
			return "";
		else
			return "Choice(0," + out + ')';
	}
	else
		throw runtime_error(ERROR_SWITCH);
}
/*
string Deserializer::deserializeBlock(const Block& block)
{
	switch (block.getIndex())
	{
	case OPTIONAL:
		return deserializeOptional(block.getValue());
	case LOOP:
		return deserializeLoop(block.getValue());
	case SWITCH:
		return deserializeSwitch(block.getValue());
	default:
		throw runtime_error("invalid keyword:" + block.getName());
	}
}*/

string Deserializer::deserializeList(const List& list)
{
	auto it = list.begin();
	string out = getSeparatedValues([&]()
	{
		return ++it != list.end();
	}, [&]()
	{
		return deserializeStuff(*it);
	});
	return out.empty() ? "" : "Sequence(" + out + ')';
}

string Deserializer::getSeparatedValues(function<bool()> condition, function<string()> output)
{
	string out(output());
	while (out.empty())
	{
		if (condition())
			out = output();
		else
			return "";
	}
	while (condition())
	{
		string temp(output());
		if (!temp.empty())
			out += CHAR_SEPARATOR + temp;
	}
	return out;
}