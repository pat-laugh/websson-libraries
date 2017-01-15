//MIT License
//Copyright(c) 2016 Patrick Laughrea
#define PatternParse(Initialization, Name, CON, END, ExtraCases, OtherValues) { \
		Initialization; \
		do \
		{ \
			switch (*it) \
			{ \
			case END: \
				checkContainerEnd(it); \
				return Name; \
			ExtraCases \
			default: \
				if (checkSeparator(it)) \
					continue; \
				OtherValues \
			} \
			checkToNextElement(it, CON); \
		} while (it); \
		throw runtime_error(ERROR_CONTAINER_NOT_CLOSED); }

#define CheckEmpty(CON, Func) \
	if (checkEmptyContainer(it, CON)) \
		Func;

#define ExtraCase(Head, Body) case Head: { Body; } break;

#define PatternOtherValues(Con, KeyValue, ValueOnly, KeyOnly, AbstractEntity) \
auto other = parseOtherValue(it, Con); \
switch (other.type) \
{ \
case OtherValue::Type::KEY_VALUE: \
	{ KeyValue; break; } \
case OtherValue::Type::VALUE_ONLY: \
	{ ValueOnly; break; } \
case OtherValue::Type::KEY_ONLY: \
	{ KeyOnly; break; } \
case OtherValue::Type::ABSTRACT_ENTITY: \
	{ AbstractEntity; break; } \
default: \
	throw logic_error(""); \
}

#define CaseKeyValue [&](std::string&& key, webss::Webss&& value)
#define CaseKeyOnly [&](std::string&& key)
#define CaseValueOnly [&](webss::Webss&& value)
#define CaseAbstractEntity [&](const webss::Entity& abstractEntity)

#define ErrorKeyValue(Message) [](std::string&& key, webss::Webss&& value) { throw std::runtime_error(Message); }
#define ErrorKeyOnly(Message) [](std::string&& key) { throw std::runtime_error(Message); }
#define ErrorValueOnly(Message) [](webss::Webss&& value) { throw std::runtime_error(Message); }
#define ErrorAbstractEntity(Message) [](const webss::Entity& abstractEntity) { throw std::runtime_error(Message); }