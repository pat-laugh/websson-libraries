//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "parser.hpp"
#include "patternsContainers.hpp"

namespace webss
{
	class Parser::ParserThead
	{
	public:
		static TheadOptions parseTheadOptions(Parser& self)
		{
			TheadOptions options;
			auto& it = self.getIt();
			while (*skipJunkToValid(++it) != CLOSE_LIST)
			{
				if (*it == CHAR_THEAD_PLUS)
					options.isPlus = true;
				else if (*it == CHAR_COLON && ++it && *it == CHAR_COLON)
					options.isText = true;
				else
					throw runtime_error(ERROR_UNEXPECTED);
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
						throw runtime_error("expand entity within binary template head must be a binary template head");
					thead.attach(content.getThead().getTheadBin());
				}
				else
					throw runtime_error("all values in a binary template must be binary");
			} while (self.checkNextElement());
			return move(thead);
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
						throw runtime_error("expand entity within standard template head must be a standard template head");
					thead.attach(content.getThead().getTheadStd());
				}
				else
				{
					self.parseExplicitKeyValue(
						CaseKeyValue{ thead.attach(move(key), move(value)); },
						CaseKeyOnly{ thead.attachEmpty(move(key)); });
				}
			} while (self.checkNextElement());
			return move(thead);
		}

	private:
		static void parseOtherValuesTheadStdAfterThead(Parser& self, TheadStd& thead)
		{
			self.parseExplicitKeyValue(
				CaseKeyValue{ thead.attach(move(key), move(value)); },
				CaseKeyOnly{ thead.attachEmpty(move(key)); });
		}

		static void parseStdParamThead(Parser& self, TheadStd& thead)
		{
			auto webssThead = self.parseThead(true);
			parseOtherValuesTheadStdAfterThead(self, thead);
			auto& lastParam = thead.back();
			auto type = webssThead.getTypeRaw();
			switch (webssThead.getTypeRaw())
			{
			case TypeThead::SELF:
				lastParam.setThead(TheadSelf());
				break;
			case TypeThead::BIN:
				lastParam.setThead(move(webssThead.getTheadBinRaw()));
				break;
			case TypeThead::STD:
				lastParam.setThead(move(webssThead.getTheadStdRaw()));
				break;
			default:
				assert(false);
			}
		}
	};
}