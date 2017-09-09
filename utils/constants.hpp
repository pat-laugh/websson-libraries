//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

namespace webss
{
	const char CHAR_ABSTRACT_ENTITY = '!';
	const char CHAR_COLON = ':';
	const char CHAR_COMMENT = '/';
	const char CHAR_CONCRETE_ENTITY = '?';
	const char CHAR_CSTRING = '"';
	const char CHAR_DECIMAL_SEPARATOR = '.';
	const char CHAR_EQUAL = '=';
	const char CHAR_ESCAPE = '\\';
	const char CHAR_EXPAND = '^';
	const char CHAR_EXPLICIT_NAME = '%';
	const char CHAR_FOREACH = '|';
	const char CHAR_IMPORT = '@';
	const char CHAR_OPTION = '#';
	const char CHAR_SCOPE = '.';
	const char CHAR_SELF = '&';
	const char CHAR_SEPARATOR = ',';
	const char CHAR_THEAD_PLUS = '+';

	const char CHAR_BIN_DEFAULT_FALSE = 0;
	const char CHAR_BIN_DEFAULT_TRUE = 1;

	const char CHAR_START_DICTIONARY = '{';
	const char CHAR_START_LIST = '[';
	const char CHAR_START_TUPLE = '(';
	const char CHAR_START_TEMPLATE = '<';
	const char CHAR_END_DICTIONARY = '}';
	const char CHAR_END_LIST = ']';
	const char CHAR_END_TUPLE = ')';
	const char CHAR_END_TEMPLATE = '>';
	
	const char CHAR_START_TEMPLATE_BIN = CHAR_START_LIST;
	const char CHAR_START_TEMPLATE_FUNCTION = CHAR_START_TUPLE;
	const char CHAR_START_TEMPLATE_OPTIONS = CHAR_START_DICTIONARY;
	const char CHAR_END_TEMPLATE_BIN = CHAR_END_LIST;
	const char CHAR_END_TEMPLATE_FUNCTION = CHAR_END_TUPLE;
	const char CHAR_END_TEMPLATE_OPTIONS = CHAR_END_DICTIONARY;
}
