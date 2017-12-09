//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#ifdef NDEBUG
#define WEBSSON_EXCEPTION(Message) Message
#else
#define WEBSSON_EXCEPTION(Message) ("<debug:" __FILE__ ":" + std::to_string(__LINE__) + "> " + std::string(Message)).c_str()
#endif

namespace webss
{
	extern const char* ERROR_ANONYMOUS_KEY;
	extern const char* ERROR_BIN_SIZE_HEAD;
	extern const char* ERROR_EXPECTED;
	extern const char* ERROR_EXPECTED_NUMBER;
	extern const char* ERROR_UNEXPECTED;
	extern const char* ERROR_VOID;
}