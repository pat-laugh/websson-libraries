//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "parser.hpp"

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
		ContainerSwitcher(Parser& parser, ConType newCon, bool newAllowVoid);
		~ContainerSwitcher();
	};

	template <class Container, ConType::Enum CON>
	Container Parser::parseContainer(Container&& cont, bool allowVoid, std::function<void(Container& cont)> func)
	{
		ContainerSwitcher switcher(*this, CON, allowVoid);
		if (!containerEmpty())
			do
				func(cont);
			while (checkNextElement());
		return std::move(cont);
	}
}