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
		static TheadOptions parseTheadOptions(Parser& self)
		{
			auto& it = self.getIt();
			TheadOptions options;
			while (*skipJunkToValid(++it) != CHAR_END_LIST)
			{
				if (*it == CHAR_THEAD_PLUS)
					options.isPlus = true;
				else if (*it == CHAR_COLON && ++it == CHAR_COLON)
					options.isText = true;
				else
					throw std::runtime_error(ERROR_UNEXPECTED);
			}
			++self.tagit;
			return options;
		}

		static TheadBin parseTheadBin(Parser& self, TheadBin&& thead = TheadBin())
		{
			do
			{
				if (*self.tagit == Tag::START_TUPLE)
					self.parseBinHead(thead);
				else if (*self.tagit == Tag::EXPAND)
				{
					const auto& content = parseExpandEntity(self.tagit, self.ents).getContent();
					if (!content.isThead() || !content.getThead().isTheadBin())
						throw std::runtime_error("expand entity within binary template head must be a binary template head");
					thead.attach(content.getThead().getTheadBin());
				}
				else
					throw std::runtime_error("all values in a binary template must be binary");
			} while (self.checkNextElement());
			return std::move(thead);
		}

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