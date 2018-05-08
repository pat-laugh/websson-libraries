//MIT License
//Copyright 2018 Patrick Laughrea

#include <iostream>
#include <fstream>

#include "parser/parser.hpp"
#include "serializer.hpp"
#include "various/smartIterator.hpp"

using namespace std;
using namespace various;
using namespace webss;

const char* NAME = "webss";

void exitErr(const char* cmd, const char* err, const char* reason);
void putErr(const char* cmd, const char* err, const char* reason);

void runFile(SmartIterator it);

int main(int argc, char** argv)
{
	if (argc <= 1)
		runFile(SmartIterator(cin.rdbuf()));
	else
		for (int i = 1; i < argc; ++i)
		{
			ifstream fileIn(argv[i], ios::binary);
			if (!fileIn)
				exitErr(NAME, (string("failed to open file ") + argv[i]).c_str(), "error");
			runFile(SmartIterator(fileIn.rdbuf()));
		}
    return 0;
}

void runFile(SmartIterator it)
{
	Document doc;
	try
		{ doc = Parser(move(it)).parseDocument(); }
	catch (const exception& e)
		{ exitErr(NAME, "parse failed", e.what()); }
	try
	{
		SerializerDynamic::serialize(doc);
	}
	catch (const exception& e)
		{ exitErr(NAME, "serialization failed", e.what()); }
}

void exitErr(const char* cmd, const char* err, const char* reason)
{
	putErr(cmd, err, reason);
	exit(EXIT_FAILURE);
}

void putErr(const char* cmd, const char* err, const char* reason)
{
	cerr << cmd << ": " << err << ": " << reason << endl;
}