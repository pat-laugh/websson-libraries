//MIT License
//Copyright 2017 Patrick Laughrea
#include "parser.hpp"

#include "containerSwitcher.hpp"
#include "errors.hpp"
#include "nameType.hpp"
#include "patternsContainers.hpp"
#include "utils/constants.hpp"

using namespace std;
using namespace webss;

const char ERROR_BINARY_TEMPLATE[] = "all values in a binary template must be binary";

TemplateHeadStandard parseTemplateHeadStandard(Parser& parser, TemplateHeadStandard&& thead = TemplateHeadStandard());
TemplateHeadBinary parseTemplateHeadBinary(Parser& parser, TemplateHeadBinary&& thead = TemplateHeadBinary());
TemplateHeadScoped parseTemplateHeadScoped(Parser& parser, TemplateHeadScoped&& thead = TemplateHeadScoped());
void parseStandardParameterTemplateHead(Parser& parser, TemplateHeadStandard& thead);
void parseOtherValuesTheadStandardAfterThead(Parser& parser, TemplateHeadStandard& thead);

Webss Parser::parseTemplateHead()
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		return BlockHead();

	switch (nextTag)
	{
	case Tag::START_TEMPLATE: case Tag::TEXT_TEMPLATE:
		return parseTemplateHeadStandard(*this);
	case Tag::START_TUPLE:
		return parseTemplateHeadBinary(*this);
	case Tag::ENTITY_ABSTRACT: case Tag::ENTITY_CONCRETE: case Tag::USING_ONE: case Tag::USING_ALL:
		return parseTemplateHeadScoped(*this);
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
		return isEnd ? move(thead) : parseTemplateHeadStandard(*this, move(thead));
	case OtherValue::Type::KEY_ONLY:
		thead.attachEmpty(move(other.key));
		return isEnd ? move(thead) : parseTemplateHeadStandard(*this, move(thead));
	case OtherValue::Type::ABSTRACT_ENTITY:
		switch (other.abstractEntity.getContent().getType())
		{
		case WebssType::BLOCK_HEAD:
			if (!isEnd)
				throw runtime_error(ERROR_UNEXPECTED);
			return BlockHead(other.abstractEntity);
		case WebssType::TEMPLATE_HEAD_BINARY:
		{
			TemplateHeadBinary theadBinary(other.abstractEntity);
			return isEnd ? move(theadBinary) : parseTemplateHeadBinary(*this, move(theadBinary));
		}
		case WebssType::TEMPLATE_HEAD_SCOPED:
		{
			TemplateHeadScoped theadScoped(other.abstractEntity);
			return isEnd ? move(theadScoped) : parseTemplateHeadScoped(*this, move(theadScoped));
		}
		case WebssType::TEMPLATE_HEAD_STANDARD:
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? move(thead) : parseTemplateHeadStandard(*this, move(thead));
		case WebssType::TEMPLATE_HEAD_TEXT:
			thead = TemplateHeadStandard(other.abstractEntity);
			return isEnd ? Webss(move(thead), true) : Webss(parseTemplateHeadStandard(*this, move(thead)), true);
		default:
			throw runtime_error("unexpected entity type within thead: " + other.abstractEntity.getContent().getType().toString());
		}
	default:
		throw runtime_error(ERROR_ANONYMOUS_KEY);
	}
}

TemplateHeadStandard Parser::parseTemplateHeadText()
{
	auto headWebss = parseTemplateHead();
	if (headWebss.getTypeRaw() == WebssType::TEMPLATE_HEAD_STANDARD || headWebss.getTypeRaw() == WebssType::TEMPLATE_HEAD_TEXT)
		return move(headWebss.getTemplateHeadStandardRaw());
	else
		throw runtime_error("expected standard template head");
}

TemplateHeadBinary parseTemplateHeadBinary(Parser& parser, TemplateHeadBinary&& thead)
{
	do
		if (parser.nextTag == Tag::START_TUPLE)
			parser.parseBinaryHead(thead);
		else if (parser.nextTag == Tag::NAME_START)
		{
			auto nameType = parseNameType(parser.getIt(), parser.getEnts());
			if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isTemplateHeadBinary())
				throw runtime_error(ERROR_BINARY_TEMPLATE);
			thead.attach(nameType.entity);
		}
		else
			throw runtime_error(ERROR_BINARY_TEMPLATE);
	while (parser.checkNextElement());
	return move(thead);
}

TemplateHeadScoped parseTemplateHeadScoped(Parser& parser, TemplateHeadScoped&& thead)
{
	set<string> entNames, nspaceNames, imports;
	do
	{
		switch (parser.nextTag)
		{
		case Tag::ENTITY_ABSTRACT:
		{
			auto ent = parser.parseAbstractEntity(Namespace::getEmptyInstance());
			containerAddSafe(entNames, string(ent.getName()));
			thead.attach(ParamScoped::makeEntityAbstract(move(ent)));
			break;
		}
		case Tag::ENTITY_CONCRETE:
		{
			auto ent = parser.parseConcreteEntity();
			containerAddSafe(entNames, string(ent.getName()));
			thead.attach(ParamScoped::makeEntityConcrete(move(ent)));
			break;
		}
		case Tag::USING_ONE:
		{
			auto param = parser.parseUsingOne();
			containerAddSafe(entNames, string(param.getEntity().getName()));
			thead.attach(move(param));
			break;
		}
		case Tag::USING_ALL:
		{
			auto param = parser.parseUsingAll();
			const auto& nspace = param.getNamespace();
			containerAddSafe(nspaceNames, string(nspace.getName()));
			thead.attach(move(param));
			break;
		}
		case Tag::IMPORT:
		{
			auto import = parser.parseImport();
			containerAddSafe(imports, string(import.getLink()));
			thead.attach(ParamScoped(import));
			break;
		}
		case Tag::NAME_START:
		{
			auto nameType = parseNameType(parser.getIt(), parser.getEnts());
			if (nameType.type != NameType::ENTITY_ABSTRACT || !nameType.entity.getContent().isTemplateHeadScoped())
				throw runtime_error(ERROR_UNEXPECTED);
			for (const auto& param : nameType.entity.getContent().getTemplateHeadScoped().getParameters())
			{
				using Type = ParamScoped::Type;
				switch (param.getType())
				{
				case Type::ENTITY_ABSTRACT: case Type::ENTITY_CONCRETE: case Type::USING_ONE:
					containerAddSafe(entNames, string(param.getEntity().getName()));
					thead.attach(ParamDocument(param));
					break;
				case Type::USING_ALL:
					containerAddSafe(nspaceNames, string(param.getNamespace().getName()));
					thead.attach(ParamDocument(param));
					break;
				case Type::IMPORT:
					containerAddSafe(imports, string(param.getImport().getLink()));
					thead.attach(ParamDocument(param));
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
	} while (parser.checkNextElement());
	return move(thead);
}

TemplateHeadStandard parseTemplateHeadStandard(Parser& parser, TemplateHeadStandard&& thead)
{
	do
		if (parser.nextTag == Tag::START_TEMPLATE)
			parseStandardParameterTemplateHead(parser, thead);
		else if (parser.nextTag == Tag::TEXT_TEMPLATE)
		{
			auto head = parser.parseTemplateHeadText();
			parseOtherValuesTheadStandardAfterThead(parser, thead);
			thead.back().setTemplateHead(move(head), true);
		}
		else
		{
			parser.parseOtherValue(
				CaseKeyValue{ thead.attach(move(key), move(value)); },
				CaseKeyOnly{ thead.attachEmpty(move(key)); },
				ErrorValueOnly(ERROR_ANONYMOUS_KEY),
				CaseAbstractEntity
				{
					if (!abstractEntity.getContent().isTemplateHeadStandard())
						throw runtime_error(ERROR_BINARY_TEMPLATE);
					thead.attach(abstractEntity);
				});
		}
	while (parser.checkNextElement());
	return move(thead);
}

void parseStandardParameterTemplateHead(Parser& parser, TemplateHeadStandard& thead)
{
	auto headWebss = parser.parseTemplateHead();
	parseOtherValuesTheadStandardAfterThead(parser, thead);
	auto& lastParam = thead.back();
	switch (headWebss.getTypeRaw())
	{
	case WebssType::BLOCK_HEAD:
		break; //do nothing
	case WebssType::TEMPLATE_HEAD_BINARY:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadBinaryRaw()));
		break;
	case WebssType::TEMPLATE_HEAD_SCOPED:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadScopedRaw()));
		break;
	case WebssType::TEMPLATE_HEAD_SELF:
		lastParam.setTemplateHead(TemplateHeadSelf());
		break;
	case WebssType::TEMPLATE_HEAD_STANDARD:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandardRaw()));
		break;
	case WebssType::TEMPLATE_HEAD_TEXT:
		lastParam.setTemplateHead(move(headWebss.getTemplateHeadStandardRaw()), true);
		break;
	default:
		assert(false);
	}
}

void parseOtherValuesTheadStandardAfterThead(Parser& parser, TemplateHeadStandard& thead)
{
	parser.checkNextElement();
	parser.parseOtherValue(
		CaseKeyValue{ thead.attach(move(key), move(value)); },
		CaseKeyOnly{ thead.attachEmpty(move(key)); },
		ErrorValueOnly(ERROR_ANONYMOUS_KEY),
		ErrorAbstractEntity(ERROR_UNEXPECTED));
}

TemplateHeadScoped Parser::parseScopedDocumentHead()
{
	ContainerSwitcher switcher(*this, ConType::TEMPLATE_HEAD, false);
	if (containerEmpty())
		throw runtime_error("can't have empty scoped document head");
	return parseTemplateHeadScoped(*this);
}