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
#include "structures/theadFun.hpp"
#include "utils/utilsWebss.hpp"

namespace webss
{
	class Parser::ParserTempl
	{
	public:
		static Webss parseTemplateBin(Parser& self, Thead thead)
		{
			Tuple body;
			auto tag = self.tagit.getSafe();
			if (tag == Tag::START_TUPLE || tag == Tag::TEXT_TUPLE)
				body = self.parseTemplateTupleBin(thead.getTheadBin().getParams(), thead.isText());
			else if (thead.isPlus())
				body = makeDefaultTuple(thead.getTheadBin().getParams());
			else
				throw std::runtime_error(WEBSSON_EXCEPTION("expected tuple"));

			if (thead.isPlus())
				return Template(std::move(thead), std::move(body), self.parseValueOnly());
			else
				return Template(std::move(thead), std::move(body));
		}

		static Webss parseTemplateStd(Parser& self, Thead thead)
		{
			Tuple body;
			WebssType typeTuple = WebssType::TUPLE;
			auto tag = self.tagit.getSafe();
			if (tag == Tag::START_TUPLE)
				body = thead.isText() ? parseTemplateTuple<true>(self, thead) : parseTemplateTuple<false>(self, thead);
			else if (tag == Tag::TEXT_TUPLE)
			{
				body = parseTemplateTuple<true>(self, thead);
				typeTuple = WebssType::TUPLE_TEXT;
			}
			else if (thead.isPlus())
				body = makeDefaultTuple(thead.getTheadStd().getParams());
			else
				throw std::runtime_error(WEBSSON_EXCEPTION("expected tuple"));

			if (thead.isPlus())
				return Template(std::move(thead), std::move(body), self.parseValueOnly(), typeTuple);
			else
				return Template(std::move(thead), std::move(body), typeTuple);
		}

		template <bool isText, bool checkDefault = true>
		static Tuple parseTemplateTuple(Parser& self, const Thead& thead)
		{
			const BasicParams<ParamStd>* paramsPtr;
			if (thead.isTheadStd())
				paramsPtr = &thead.getTheadStd().getParams();
			else
			{
				assert(thead.isTheadFun());
				paramsPtr = &thead.getTheadFun().getThead().getParams();
			}
			const auto& params = *paramsPtr;

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
						tuple.at(index) = parseLineString(self);
					else
						parseTemplateTupleName(self, thead, tuple, index);
					break;
				default:
					tuple.at(index) = isText ? parseLineString(self) : parseTemplateContainer(self, thead, params.at(index));
					break;
				}
				++index;
			});
			if (checkDefault)
				checkDefaultValues(tuple, params);
			return tuple;
		}

		static void foreachList(Parser& self, const Thead& thead, const List& from, List& list)
		{
			for (decltype(from.size()) i = 0; i < from.size(); ++i)
			{
				const auto& item = from[i];
				auto type = item.getTypeRaw();
				if (type == WebssType::TUPLE || type == WebssType::TUPLE_TEXT)
				{
					//associate tuple to template head
					Tuple tuple;
					const auto& params = thead.isTheadStd() ? thead.getTheadStd().getParams() : thead.getTheadFun().getThead().getParams();
					fillTemplateTuple(self, params, item.getTupleRaw(), tuple);

					if (!thead.isPlus())
						list.add(Template(thead, std::move(tuple), type));
					else
					{
						//get next item as body
						if (++i == from.size())
							throw std::runtime_error(WEBSSON_EXCEPTION(ERROR_VOID));
						list.add(Template(thead, std::move(tuple), from[i], type));
					}
				}
				else if (!thead.isPlus())
					throw std::runtime_error(WEBSSON_EXCEPTION("regular template must be implemented"));
				else
					list.add(Template(thead, Tuple(), item));
			}
		}

		static void foreachValue(Parser& self, const Thead& thead, Webss&& value, List& list)
		{
			switch (value.getType())
			{
			case WebssType::LIST: case WebssType::LIST_TEXT:
				foreachList(self, thead, value.getList(), list);
				break;
			case WebssType::TEMPLATE:
			{
				const auto& templ = value.getTemplate();
				if (templ.isPlus())
				{
					if (!templ.content.isList())
						throw std::runtime_error(WEBSSON_EXCEPTION("expected list"));
					foreachList(self, thead, templ.content.getList(), list);
				}
				else if (templ.isTheadFun())
				{
					const auto& theadFun = templ.getTheadFun();
					if (!theadFun.getStructure().isList())
						throw std::runtime_error(WEBSSON_EXCEPTION("expected list"));
					const auto& from = theadFun.getStructure().getList();
					const auto& tuple = templ.body.getTuple();
					for (decltype(from.size()) i = 0; i < from.size(); ++i)
					{
					//	const auto& item = from[i];
					//	auto type = item.getTypeRaw();
						if (!thead.isPlus())
							throw std::runtime_error(WEBSSON_EXCEPTION("template head with foreach template function be plus"));
						else
							list.add(Template(thead, Tuple(), Template(TheadFun(theadFun, i), tuple)));
					}
				}
				else
					throw std::runtime_error(WEBSSON_EXCEPTION("expected list"));
				break;
			}
			default:
				throw std::runtime_error(WEBSSON_EXCEPTION("expected list"));
			}
		}

	private:
		static void parseTemplateTupleName(Parser& self, const Thead& thead, Tuple& tuple, Tuple::size_type& index)
		{
			const BasicParams<ParamStd>* paramsPtr;
			if (thead.isTheadStd())
				paramsPtr = &thead.getTheadStd().getParams();
			else
			{
				assert(thead.isTheadFun());
				paramsPtr = &thead.getTheadFun().getThead().getParams();
			}
			const auto& params = *paramsPtr;

			auto nameType = parseNameType(self.tagit, self.ents);
			if (nameType.type != NameType::NAME && params.at(index).hasThead())
				throw std::runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
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
					throw std::runtime_error(WEBSSON_EXCEPTION(ERROR_UNEXPECTED));
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
			default: assert(false);
			case TypeThead::SELF:
				return self.parseTemplateBody(thead);
			case TypeThead::BIN: case TypeThead::FUN: case TypeThead::STD:
				return self.parseTemplateBody(param.getThead());
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
				throw std::runtime_error(WEBSSON_EXCEPTION("can't expand tuple for template head plus"));
			switch (param.getTypeThead())
			{
			default: assert(false);
			case TypeThead::SELF:
				return buildTemplate(self, params, tupleItem);
			case TypeThead::BIN:
				throw std::runtime_error(WEBSSON_EXCEPTION("can't expand for a binary template"));
			case TypeThead::STD:
				return buildTemplate(self, param.getTheadStd().getParams(), tupleItem);
			}
		}

		static Webss buildTemplate(Parser& self, const TheadStd::Params& params, const Webss& templateItem)
		{
			if (!templateItem.isTuple())
				throw std::runtime_error(WEBSSON_EXCEPTION("template head must be implemented"));
			return buildTemplateTuple(self, params, templateItem.getTuple());
		}
	};
}