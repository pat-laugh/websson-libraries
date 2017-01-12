//MIT License
//Copyright(c) 2016 Patrick Laughrea
#define PatternParse(Initialization, Name, CON, END, ExtraCases, AfterSeparator) { \
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
				AfterSeparator \
			} \
			checkToNextElement(it, CON); \
		} while (it); \
		throw runtime_error(ERROR_CONTAINER_NOT_CLOSED); }

#define CheckEmpty(CON, Func) \
	if (checkEmptyContainer(it, CON)) \
		Func;

#define ExtraCase(Head, Body) case Head: { Body; } break;

#define CheckOtherValues(NameStart, NumberStart) { \
	if (isNameStart(*it)) { NameStart; } \
	else if (isNumberStart(*it)) { NumberStart; } \
	else throw runtime_error(ERROR_UNEXPECTED); }

#define PatternParseNameStart(Container, CON, CaseNamed, CaseAnonymous) \
	void parseNameStart##Container(It& it, Container& cont) \
	{ \
		auto keyPair = parseKey(it); \
		switch (keyPair.second) \
		{ \
		case webss_KEY_TYPE_ANY_CONTAINER_CHAR_VALUE: \
			CaseNamed; \
			break; \
		CaseAnonymous \
		default: \
			throw runtime_error(ERROR_UNEXPECTED); \
		} \
	}

#define ExtraCasesAnonymous(CON) \
	case KeyType::KEYWORD: \
		cont.add(Keyword(keyPair.first)); \
		break; \
	case KeyType::KEYNAME: \
		throw runtime_error(webss_ERROR_UNDEFINED_KEYNAME(keyPair.first)); \
	case KeyType::VARIABLE: \
		cont.add(checkIsConcrete(vars[keyPair.first])); \
		break; \
	case KeyType::SCOPE: \
		cont.add(checkIsConcrete(parseScopedValue(it, keyPair.first))); \
		break; \
	case KeyType::BLOCK_VALUE: \
		cont.add(parseBlockValue(it, CON, keyPair.first)); \
		break;