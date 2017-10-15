//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "errors.hpp"
#include "parser.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"

namespace webss
{
	class Parser::ParserThead
	{
	public:
		static TheadStd parseTheadStd(Parser& self, TheadStd&& thead = TheadStd())
		{
			do
			{
				if (*self.tagit == Tag::START_TEMPLATE)
					parseStdParamThead(self, thead);
				else if (*self.tagit == Tag::EXPAND)
				{
					const auto& content = parseExpandEntity(self.tagit, self.ents).getContent();
					if (!content.isThead() || !content.getThead().isTheadStd())
						throw std::runtime_error("expand entity within standard template head must be a standard template head");
					thead.attach(content.getThead().getTheadStd());
				}
				else
				{
					self.parseExplicitKeyValue(
						CaseKeyValue{ thead.attach(std::move(key), std::move(value)); },
						CaseKeyOnly{ thead.attachEmpty(std::move(key)); });
				}
			} while (self.checkNextElement());
			return std::move(thead);
		}

	private:
		static void parseStdParamThead(Parser& self, TheadStd& thead)
		{
			auto paramThead = self.parseThead(true);
			self.parseExplicitKeyValue(
				CaseKeyValue{ thead.attach(std::move(key), std::move(value)); },
				CaseKeyOnly{ thead.attachEmpty(std::move(key)); });
			thead.back().setThead(std::move(paramThead));
		}
	};
}