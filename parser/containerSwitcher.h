//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "parser.h"

namespace webss
{
	class Parser::ContainerSwitcher
	{
	private:
		Parser& parser;
		ConType oldCon;
		bool oldAllowVoid;
		bool oldMultilineContainer;
	public:
		ContainerSwitcher(Parser& parser, ConType newCon, bool newAllowVoid) : parser(parser), oldCon(parser.con), oldAllowVoid(parser.allowVoid), oldMultilineContainer(parser.multilineContainer)
		{
			parser.con = newCon;
			parser.allowVoid = newAllowVoid;
			parser.multilineContainer = checkLineEmpty(++parser.it);
		}

		~ContainerSwitcher()
		{
			parser.con = oldCon;
			parser.allowVoid = oldAllowVoid;
			parser.multilineContainer = oldMultilineContainer;
		}
	};

	template <class Container, ConType::Enum CON>
	Container Parser::parseContainer(Container&& cont, std::function<void(Container& cont)> func)
	{
		ContainerSwitcher switcher(*this, CON, false);
		if (!containerEmpty())
			do
				func(cont);
		while (checkNextElement());
		return std::move(cont);
	}
}