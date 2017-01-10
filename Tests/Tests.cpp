// Tests.cpp : Defines the entry point for the console application.
//

#include <iostream>
#include <fstream>

#include "stdafx.h"
#include "WebssonParser/parser.h"
#include "WebssonDeserializer/deserializer.h"
//#include "Person.h"
//#include "Date.h"

using namespace std;
using namespace webss;

int main()
{
	char inChar;
	/*
	{
	string filename("webssDateBinary");
	Parser parser;
	Webss data;
	string path("C:\\Users\\Pat-Laugh\\Desktop\\");
	string filenameIn = filename + ".txt";
	string filenameOut = filename + "Out.txt";
	string fileInName(path + filenameIn);
	string fileOutName(path + filenameOut);

	cout << endl << fileInName << endl;

	ifstream fileIn(fileInName, ios::binary);
	if (fileIn.fail()) { cerr << "Error: failed to open file \"" << fileInName << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
	try
	{
	const auto& webss = parser.parse(fileIn);
	Date date(webss[0].getTuple());
	cout << "No errors while parsing" << endl;

	//	Date date(2016, 10, 20);
	ofstream fileOut(fileOutName, ios::binary);
	if (fileOut.fail()) { cerr << "Error: failed to open file \"" << fileOutName << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
	try
	{
	Tuple tuple;
	tuple.add(date.deserialize());
	tuple.add(date.deserializeRegular());
	fileOut << parser.deserializeAll(tuple);
	//	fileOut << parser.deserializeAll();
	fileOut.flush();
	cout << "No errors while deserializing" << endl;
	}
	catch (exception e)
	{
	cout << "Deserialization failed: " << e.what() << endl;
	}
	fileOut.close();
	}
	catch (exception e)
	{
	cout << "Parse failed: " << e.what() << endl;
	}
	fileIn.close();


	}

	cin >> inChar;
	return 0;*/

	/*{
	string filename("webssPersonWholeSchema");
	Parser parser;
	Webss data;
	string path("C:\\Users\\Pat-Laugh\\Desktop\\");
	string filenameIn = filename + ".txt";
	string filenameOut = filename + "Out.txt";
	string fileInName(path + filenameIn);
	string fileOutName(path + filenameOut);

	cout << endl << fileInName << endl;

	ifstream fileIn(fileInName, ios::binary);
	if (fileIn.fail()) { cerr << "Error: failed to open file \"" << fileInName << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
	try
	{
	const auto& webss = parser.parse(fileIn);
	Person person1(webss[0].getTuple()), person2(webss[1].getTuple());
	cout << "No errors while parsing" << endl;

	ofstream fileOut(fileOutName, ios::binary);
	if (fileOut.fail()) { cerr << "Error: failed to open file \"" << fileOutName << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
	try
	{
	Tuple tuple;
	tuple.add(person1.deserialize(parser.vars["Schema"]));
	tuple.add(person2.deserialize(parser.vars["Schema"]));
	fileOut << parser.deserializeAll(tuple);
	fileOut.flush();
	cout << "No errors while deserializing" << endl;
	}
	catch (exception e)
	{
	cout << "Deserialization failed: " << e.what() << endl;
	}
	fileOut.close();
	}
	catch (exception e)
	{
	cout << "Parse failed: " << e.what() << endl;
	}
	fileIn.close();

	cin >> inChar;
	}

	return 0; */

	{
		string filename("testWebsson");
		Parser parser;
		Document data;
		string path("C:\\Users\\Pat-Laugh\\Desktop\\");
		string filenameIn = filename + ".txt";
		string filenameOut = filename + "Out.txt";
		string fileInName(path + filenameIn);
		string fileOutName(path + filenameOut);

		cout << endl << fileInName << endl;

		ifstream fileIn(fileInName, ios::binary);
		if (fileIn.fail()) { cerr << "Error: failed to open file \"" << fileInName << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
		try
		{
			data = parser.parse(fileIn);
			cout << "No errors while parsing" << endl;

			ofstream fileOut(fileOutName, ios::binary);
			if (fileOut.fail()) { cerr << "Error: failed to open file \"" << fileOutName << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
			try
			{
				fileOut << deserializeAll(data, parser.vars);
				fileOut.flush();
				cout << "No errors while deserializing" << endl;
			}
			catch (exception e)
			{
				cout << "Deserialization failed: " << e.what() << endl;
			}
			fileOut.close();
		}
		catch (exception e)
		{
			cout << "Parse failed: " << e.what() << endl;
		}
		fileIn.close();

		cin >> inChar;
	}
	return 0;
}