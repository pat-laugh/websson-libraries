//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "WebssonParser/parser.h"

class Deserializer
{
private:
	enum { OPTIONAL, LOOP, SWITCH };
public:
	static std::string makeDiagram(const std::string& in) { return Deserializer().getDeserializedStuff(getParser().parse(in)); }
	static std::string makeDiagram(std::istream& in) { return Deserializer().getDeserializedStuff(getParser().parse(in)); }

	static std::string makeDiagram(webss::Parser& parser, const std::string& in) { return Deserializer().getDeserializedStuff(parser.parse(in)); }
	static std::string makeDiagram(webss::Parser& parser, std::istream& in) { return Deserializer().getDeserializedStuff(parser.parse(in)); }

	static webss::Parser getParser()
	{
		webss::Parser parser;
		parser.addBlock("optional", OPTIONAL);
		parser.addBlock("loop", LOOP);
		parser.addBlock("switch", SWITCH);
		return parser;
	}
private:
	Deserializer() {}

	std::string getDeserializedStuff(webss::Webss&& webss)
	{
		const auto& tuple = webss.getTuple();
		auto it = tuple.begin();
		return "Diagram(" + getSeparatedValues([&]() { return ++it != tuple.end(); }, [&]() { return deserializeStuff(*it); }) + ')';
	}

	std::string deserializeStuff(const webss::Webss& webss);
	std::string deserializeString(const std::string& s);
	std::string deserializeTuple(const webss::Tuple& tuple);
	std::string deserializeBlock(const webss::Block& block);
	std::string deserializeList(const webss::List& list);
	std::string getSeparatedValues(std::function<bool()> condition, std::function<std::string()> output);

	std::string deserializeSwitch(const webss::Webss& value);
	std::string deserializeLoop(const webss::Webss& value);
	std::string deserializeOptional(const webss::Webss& value);
	std::string deserializeLoopParam(const webss::Webss& value);
};