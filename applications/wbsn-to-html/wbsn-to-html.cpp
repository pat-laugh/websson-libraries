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

int main(int argc, char** argv)
{
	SmartIterator it("");
	if (argc <= 1)
		it = SmartIterator(cin.rdbuf());
	else if (argc > 3)
	{
		cout << "Usage: " << argv[0] << " [fileIn] [fileOut]" << endl;
		return 1;
	}
	else
	{
		ifstream fileIn(argv[1], ios::binary);
		if (fileIn.fail())
		{
			cout << "Error: failed to open file \"" << argv[1] << "\"" << endl;
			return 1;
		}
		it = SmartIterator(fileIn.rdbuf());
	}
	try
	{
		auto doc = Parser(move(it)).parseDocument();
		if (argc <= 2)
			cout << SerializerHtml::serialize(doc);
		else
		{
			ofstream fileOut(argv[2], ios::binary);
			if (fileOut.fail())
			{
				cout << "Error: failed to open file \"" << argv[2] << "\"" << endl;
				return 1;
			}
			fileOut << SerializerHtml::serialize(doc);
		}
	} catch (const exception& e)
	{
		cerr << "Error: " << e.what() << endl;
		return 1;
	}
		
    return 0;
}

