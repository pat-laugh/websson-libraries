//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "parser.h"
#include "patternsContainers.h"

using namespace std;
using namespace webss;

Entity Parser::parseConcreteEntity(It& it, ConType con)
{
	Entity ent;
	parseOtherValue(it, con,
		CaseKeyValue{ ent = Entity(move(key), move(value)); },
		CaseKeyOnly{ throw runtime_error(ERROR_EXPECTED); },
		CaseValueOnly{ throw runtime_error(ERROR_UNEXPECTED); },
		CaseAbstractEntity{ throw runtime_error(webss_ERROR_ENTITY_EXISTS(abstractEntity.getName())); });
	return ent;
}

Entity Parser::parseAbstractEntity(It& it, const Namespace& currentNamespace)
{
	auto name = parseNameSafe(it);
	switch (*skipJunkToValid(it))
	{
	case OPEN_DICTIONARY:
		return Entity(move(name), parseNamespace(++it, name, currentNamespace));
	case OPEN_LIST:
		return Entity(move(name), Webss(parseEnum(++it, name), true));
	case OPEN_FUNCTION:
		return Entity(move(name), parseFunctionHead(++it));
	case CHAR_COLON:
		if (++it != CHAR_COLON || skipJunk(++it) != OPEN_FUNCTION)
			throw runtime_error("expected text function head");
		return Entity(move(name), parseFunctionHeadText(++it));
	default:
		throw runtime_error(ERROR_UNEXPECTED);
	}
}

string Parser::parseName(It& it)
{
	string name;
	do
		name += *it;
	while (++it && isNameBody(*it));
	return name;
}

string Parser::parseNameSafe(It& it)
{
	skipJunkToValidCondition(it, [&]() { return isNameStart(*it); });
	auto nameType = parseNameType(it);
	if (nameType.type == NameType::NAME)
		return move(nameType.name);
	throw runtime_error("expected name that is neither an entity nor a keyword");
}

ScopedDocument Parser::parseScopedDocument(It& it)
{
	static const ConType CON = ConType::DICTIONARY;
	static const auto& currentNamespace = Namespace::getEmptyInstance();
	if (*it != OPEN_FUNCTION)
		throw runtime_error(ERROR_UNEXPECTED);
	auto head = parseFunctionHeadScoped(++it);

	skipJunkToValidCondition(it, [&]() { return *it == OPEN_DICTIONARY; });
	DocumentHead body;
	if (checkEmptyContainer(++it, CON))
		return{ move(head), move(body) };

	vector<Entity> entitiesToReAdd;

	//get ents
	const auto& params = head.getParameters();
	for (const auto& param : params)
		if (param.hasEntity())
			ents.addLocalSafe(param.getEntity());
		else
		{
			const auto& nspace = param.getNamespace();
			const auto& name = nspace.getName();
			if (ents.hasEntity(name))
			{
				const auto& ent = ents.getWebss(name);
				const auto& content = ent.getContent();
				if (content.isNamespace() && content.getNamespace().getPointer() == nspace.getPointer())
				{
					entitiesToReAdd.push_back(ent);
					ents.removeLocal(name);
				}
			}
			for (const auto& ent : nspace)
				ents.addLocalSafe(ent);
		}

	do
	{
		switch (*it)
		{
		case CHAR_ABSTRACT_ENTITY:
			checkMultiContainer(++it, [&]() { auto ent = parseAbstractEntity(it, currentNamespace); body.push_back(ParamDocument(ent)); ents.addLocal(move(ent)); });
			break;
		case CHAR_CONCRETE_ENTITY:
			checkMultiContainer(++it, [&]() { auto ent = parseConcreteEntity(it, CON); body.push_back(ParamDocument(ent, true)); ents.addLocal(move(ent)); });
			break;
		case CHAR_USING_NAMESPACE:
			checkMultiContainer(++it, [&]() { body.push_back(parseScopedDocument(it)); });
			break;
		case CHAR_IMPORT:
			checkMultiContainer(++it, [&]() { parseImport(it); });
			break;
		default:
			throw runtime_error(ERROR_UNEXPECTED);
		}
	} while (checkNextElementContainer(it, CON));

	//remove ents
	for (const auto& param : params)
		if (param.hasEntity())
			ents.removeLocal(param.getEntity());
		else
			for (const auto& ent : param.getNamespace())
				ents.removeLocal(ent);

	for (const auto& ent : entitiesToReAdd)
		ents.addLocal(ent);

	return{ move(head), move(body) };
}