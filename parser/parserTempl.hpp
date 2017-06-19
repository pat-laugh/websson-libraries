//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "parser.hpp"
#include "parserStrings.hpp"
#include "patternsContainers.hpp"
#include "utilsExpand.hpp"
#include "utilsTemplateDefaultValues.hpp"
#include "utils/utilsWebss.hpp"

namespace webss
{
	const char ERROR_EXPAND_BIN_TEMPLATE[] = "can't expand for a binary template";

	class Parser::ParserTempl
	{
	public:
		template <bool isText>
		static Tuple parseTemplateTuple(Parser& self, const Thead& thead)
		{
			const auto params = thead.getTheadStd().getParams();
			Tuple::size_type index = 0;
			Tuple tuple = self.parseContainer<Tuple, ConType::TUPLE>(Tuple(params.getSharedKeys()), true, [&self, &params, &thead, &index](Tuple& tuple)
			{
				switch (*self.tagit)
				{
				case Tag::SEPARATOR: //void
					break;
				case Tag::EXPAND:
					index = fillTemplateTuple(self, params, parseExpandTuple(self.tagit, self.ents), tuple, index);
					return;
				case Tag::EXPLICIT_NAME:
				{
					auto name = parseNameExplicit(self.tagit);
					tuple.at(name) = isText ? Webss(self.parseValueOnly()) : parseTemplateContainer(self, thead, params.at(name));
					break;
				}
				case Tag::NAME_START:
					if (isText)
						tuple.at(index) = Webss(parseLineString(self));
					else
						parseTemplateTupleName(self, thead, tuple, index);
					break;
				default:
					tuple.at(index) = isText ? Webss(parseLineString(self)) : parseTemplateContainer(self, thead, params.at(index));
					break;
				}
				++index;
			});
			checkDefaultValues(tuple, params);
			return tuple;
		}

	private:
		static void parseTemplateTupleName(Parser& self, const Thead& thead, Tuple& tuple, Tuple::size_type& index)
		{
			const auto params = thead.getTheadStd().getParams();
			auto nameType = parseNameType(self.tagit, self.ents);
			if (nameType.type != NameType::NAME && params.at(index).hasThead())
				throw std::runtime_error(ERROR_UNEXPECTED);
			switch (nameType.type)
			{
			case NameType::NAME:
				tuple.at(nameType.name) = parseTemplateContainer(self, thead, params.at(nameType.name));
				break;
			case NameType::KEYWORD:
				tuple.at(index) = std::move(nameType.keyword);
				break;
			case NameType::ENTITY_ABSTRACT:
			{
				auto otherValue = self.checkAbstractEntity(nameType.entity);
				if (otherValue.type != OtherValue::VALUE_ONLY)
					throw std::runtime_error(ERROR_UNEXPECTED);
				tuple.at(index) = std::move(otherValue.value);
				break;
			}
			case NameType::ENTITY_CONCRETE:
				tuple.at(index) = std::move(nameType.entity);
				break;
			}
		}

		static Webss parseTemplateContainer(Parser& self, const Thead& thead, const ParamStd& param)
		{
			if (!param.hasThead())
				return self.parseValueOnly();
			switch (param.getTypeThead())
			{
			case TypeThead::SELF:
				return self.parseTemplateStd(thead);
			case TypeThead::BIN:
				return self.parseTemplateBin(param.getThead());
			case TypeThead::STD:
				return self.parseTemplateStd(param.getThead());
			default:
				assert(false); throw std::domain_error("");
			}
		}

		//creates a tuple and fills it
		static Tuple buildTemplateTuple(Parser& self, const TheadStd::Params& params, const Tuple& tupleToCopy)
		{
			Tuple templateTuple(params.getSharedKeys());
			if (fillTemplateTuple(self, params, tupleToCopy, templateTuple) < templateTuple.size())
				checkDefaultValues(templateTuple, params);
			return templateTuple;
		}

		//fills an existing tuple starting at index; returns index + number of values copied
		static Tuple::size_type fillTemplateTuple(Parser& self, const TheadStd::Params& params, const Tuple& tupleToCopy, Tuple& tupleToFill, Tuple::size_type index = 0)
		{
			for (const auto& item : tupleToCopy.getOrderedKeyValues())
			{
				if (item.first == nullptr)
					tupleToFill.at(index) = checkTemplateContainer(self, params, params.at(index), *item.second);
				else
					tupleToFill.at(*item.first) = checkTemplateContainer(self, params, params.at(*item.first), *item.second);
				++index;
			}
			return index;
		}

		static Webss checkTemplateContainer(Parser& self, const TheadStd::Params& params, const ParamStd& param, const Webss& tupleItem)
		{
			if (!param.hasThead())
				return tupleItem;
			if (param.isPlusThead())
				throw std::runtime_error("can't expand tuple for template head plus");
			switch (param.getTypeThead())
			{
			case TypeThead::SELF:
				return buildTemplate(self, params, tupleItem);
			case TypeThead::BIN:
				throw std::runtime_error(ERROR_EXPAND_BIN_TEMPLATE);
			case TypeThead::STD:
				return buildTemplate(self, param.getTheadStd().getParams(), tupleItem);
			default:
				assert(false); throw std::domain_error("");
			}
		}

		static Webss buildTemplate(Parser& self, const TheadStd::Params& params, const Webss& templateItem)
		{
			if (!templateItem.isTuple())
				throw std::runtime_error("template head must be implemented");
			return buildTemplateTuple(self, params, templateItem.getTuple());
		}
	};
}