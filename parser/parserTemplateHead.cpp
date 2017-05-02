//MIT License
//Copyright(c) 2017 Patrick Laughrea
#include "parser.h"

#include "errors.h"
#include "patternsContainers.h"
#include "utils/constants.h"

using namespace std;
using namespace webss;

const char ERROR_BINARY_TEMPLATE[] = "all values in a binary template must be binary";

Webss Parser::parseTemplateHead()
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		return BlockHead();

	switch (nextTag)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
		return parseTemplateHeadStandard();
	case Tag::START_TUPLE:
		return parseTemplateHeadBinary();
	case Tag::ENTITY_ABSTRACT: case Tag::ENTITY_CONCRETE: case Tag::USING_ONE: case Tag::USING_ALL:
		return parseTemplateHeadScoped();
	case Tag::SELF:
		skipJunkToTag(++it, Tag::END_TEMPLATE);
		++it;
		return TemplateHeadSelf();
	default:
		break;
	}

	//if it's a entity, then the thead is of the same type as the entity
	//if not, then the thead is a standard thead

	TemplateHeadStandard thead;
	auto other = parseOtherValue();
	bool isEnd = !checkNextElement();
	switch (other.type)
	{
	case OtherValue::Type::KEY_VALUE:
		thead.attach(move(other.key), move(other.value));
		return isEnd ? move(thead) : parseTemplateHeadStandard(move(thead));
	case OtherValue::Type::KEY_ONLY:
		thead.attachEmpty(move(other.key));
		return isEnd ? move(thead) : parseTemplateHeadStandard(move(thead));
	case OtherValue::Type::ABSTRACT_ENTITY:
		switch (other.abstractEntity.getContent().getTypeSafe())
		{
		case WebssType::BLOCK_HEAD:
			if (!isEnd)
				throw runtime_error(ERROR_UNEXPECTED);
			return BlockHead(other.abstractEntity);
		case WebssType::TEMPLATE_HEAD_BINARY:
		{
			TemplateHeadBinary theadBinary(other.abstractEntity);
			return isEnd ? move(theadBinary) : parseTemplateHeadBinary(move(theadBinary));
		}
		case WebssType::TEMPLATE_HEAD_SCOPED:
		{
			TemplateHeadScoped theadScoped(other.abstractEntity);
			return isEnd ? move(theadScoped) : parseTemplateHeadScoped(move(theadScoped));
		}
		case WebssType::TEMPLATE_HEAD_STANDARD:
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? move(thead) : parseTemplateHeadStandard(move(thead));
		case WebssType::TEMPLATE_HEAD_TEXT:
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? Webss(move(thead), true) : Webss(parseTemplateHeadStandard(move(thead)), true);
		default:
			throw runtime_error("unexpected entity type within thead: " + other.abstractEntity.getContent().getTypeSafe().toString());
		}
	default:
		throw runtime_error(ERROR_ANONYMOUS_KEY);
	}
}

TemplateHeadStandard Parser::parseTemplateHeadText()
{
	auto headWebss = parseTemplateHead();
	if (headWebss.getType() == WebssType::TEMPLATE_HEAD_STANDARD || headWebss.getType() == WebssType::TEMPLATE_HEAD_TEXT)
		return move(headWebss.getTemplateHeadStandard());
	else
		throw runtime_error("expected standard template head");
}

TemplateHeadBinary Parser::parseTemplateHeadBinary(TemplateHeadBinary&& thead)
{
	assert(it);
	do
		if (nextTag == Tag::START_TUPLE)
			parseBinaryHead(thead);
		else if (nextTag == Tag::NAME_START)
		{
			auto nameType = parseNameType();
			if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isTemplateHeadBinary())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(nameType.entity);
		}
		else
			throw runtime_error(ERROR_BINARY_TEMPLATE);
	while (checkNextElement());
	return move(thead);
}

TemplateHeadScoped Parser::parseTemplateHeadScoped(TemplateHeadScoped&& thead)
{
	set<string> entNames, nspaceNames;
	assert(it);
	do
	{
		switch (nextTag)
		{
		case Tag::ENTITY_ABSTRACT:
		{
			auto ent = parseAbstractEntity(Namespace::getEmptyInstance());
			containerAddSafe(entNames, string(ent.getName()));
			thead.attach(ParamScoped::makeEntityAbstract(move(ent)));
			break;
		}
		case Tag::ENTITY_CONCRETE:
		{
			auto ent = parseConcreteEntity();
			containerAddSafe(entNames, string(ent.getName()));
			thead.attach(ParamScoped::makeEntityConcrete(move(ent)));
			break;
		}
		case Tag::USING_ONE:
		{
			auto ent = parseUsingOne();
			containerAddSafe(entNames, string(ent.getName()));
			thead.attach(ParamScoped::makeUsingOne(move(ent)));
			break;
		}
		case Tag::USING_ALL:
		{
			const auto& nspace = parseUsingAll();
			containerAddSafe(nspaceNames, string(nspace.getName()));
			thead.attach(ParamScoped(nspace));
			break;
		}
		case Tag::NAME_START:
		{
			auto nameType = parseNameType();
			if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isTemplateHeadScoped())
				throw runtime_error(ERROR_UNEXPECTED);
			for (const auto& param : nameType.entity.getContent().getTemplateHeadScopedSafe().getParameters())
			{
				switch (param.getType())
				{
				case ParamScoped::Type::ENTITY_ABSTRACT:
					containerAddSafe(entNames, string(param.getAbstractEntity().getName()));
					thead.attach(ParamScoped::makeEntityAbstract(param.getAbstractEntity()));
					break;
				case ParamScoped::Type::ENTITY_CONCRETE:
					containerAddSafe(entNames, string(param.getConcreteEntity().getName()));
					thead.attach(ParamScoped::makeEntityConcrete(param.getConcreteEntity()));
					break;
				case ParamScoped::Type::USING_ONE:
					//TODO
				//	containerAddSafe(nspaceNames, string(param.getNamespace().getName()));
				//	thead.attach(ParamScoped(param.getNamespace()));
				//	break;
				case ParamScoped::Type::USING_ALL:
					containerAddSafe(nspaceNames, string(param.getNamespace().getName()));
					thead.attach(ParamScoped(param.getNamespace()));
					break;
				default:
					assert(false);
				}
			}
			break;
		}
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}	
	} while (checkNextElement());
	return move(thead);
}

TemplateHeadStandard Parser::parseTemplateHeadStandard(TemplateHeadStandard&& thead)
{
	assert(it);
	do
		if (nextTag == Tag::START_TEMPLATE)
			parseStandardParameterTemplateHead(thead);
		else if (nextTag == Tag::TEXT_TEMPLATE)
		{
			auto head = parseTemplateHeadText();
			parseOtherValuesTheadStandardAfterThead(thead);
			thead.back().setTemplateHead(move(head), true);
		}
		else
		{
			parseOtherValue(
				CaseKeyValue{ thead.attach(move(key), move(value)); },
				CaseKeyOnly{ thead.attachEmpty(move(key)); },
				CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
				CaseAbstractEntity
				{
					if (!abstractEntity.getContent().isTemplateHeadStandard())
						throw runtime_error(ERROR_BINARY_TEMPLATE);
					thead.attach(abstractEntity);
				});
		}
	while (checkNextElement());
	return move(thead);
}

void Parser::parseStandardParameterTemplateHead(TemplateHeadStandard& thead)
{
	auto headWebss = parseTemplateHead();
	parseOtherValuesTheadStandardAfterThead(thead);
	auto& lastParam = thead.back();
	switch (headWebss.getType())
	{
	case WebssType::BLOCK_HEAD:
		break; //do nothing
	case WebssType::TEMPLATE_HEAD_BINARY:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadBinary()));
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadScoped()));
		break;
	case WebssType::TEMPLATE_HEAD_SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandard()));
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandard()), true);
		break;
	default:
		throw logic_error("");
	}
}

void Parser::parseOtherValuesTheadStandardAfterThead(TemplateHeadStandard& thead)
{
	nextTag = getTag(it);
	parseOtherValue(
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		CaseValueOnly{ throw runtime_error(ERROR_ANONYMOUS_KEY); },
		CaseAbstractEntity{ throw runtime_error(ERROR_UNEXPECTED); });
}

