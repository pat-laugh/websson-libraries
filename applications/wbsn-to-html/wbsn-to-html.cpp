//MIT License
//Copyright 2017 Patrick Laughrea

#include <iostream>
#include <fstream>

#include "parser/parser.hpp"
#include "serializerHtml/serializerHtml.hpp"
#include "various/smartIterator.hpp"

using namespace std;
using namespace various;
using namespace webss;

void exitErr(const char* cmd, const char* err, const char* reason);
void putErr(const char* cmd, const char* err, const char* reason);

int main(int argc, char** argv)
{
	SmartIterator it("");
	if (argc <= 1)
		it = SmartIterator(cin.rdbuf());
	else if (argc > 3)
	{
		cerr << "Usage: " << argv[0] << " [fileIn] [fileOut]" << endl;
		return EXIT_FAILURE;
	}
	else
	{
		ifstream fileIn(argv[1], ios::binary);
		if (!fileIn)
			exitErr(argv[0], (string("failed to open file ") + argv[1]).c_str(), "error");
		it = SmartIterator(fileIn.rdbuf());
	}
	
	Document doc;
	try
		{ doc = Parser(move(it), argv[1]).parseDocument(); }
	catch (const exception& e)
		{ exitErr(argv[0], "parse failed", e.what()); }
	try
	{
		if (argc <= 2)
		{
			cout << SerializerHtml::serialize(doc);
			cout.flush();
		}
		else
		{
			ofstream fileOut(argv[2], ios::binary);
			if (fileOut.fail())
				exitErr(argv[0], (string("failed to open file ") + argv[2]).c_str(), "error");
			fileOut << SerializerHtml::serialize(doc);
			fileOut.flush();
		}
	}
	catch (const exception& e)
		{ exitErr(argv[0], "serialization failed", e.what()); }
		
    return 0;
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