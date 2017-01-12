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

#define CheckOtherValues(NameStart, NumberStart) { \
	if (isNameStart(*it)) { NameStart; } \
	else if (isNumberStart(*it)) { NumberStart; } \
	else throw runtime_error(ERROR_UNEXPECTED); }