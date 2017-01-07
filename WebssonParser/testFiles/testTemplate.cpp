#include <iostream>
#include <fstream>
#include "webssonSingleHeader.h"

using namespace std;
using namespace webss;

int main()
{
	string path("pathToFile");
	string filename("testWebsson");
	
	string ext("txt");
	string outNameAppend("Out");

	char inChar;
	
	Parser parser;
	Webss data;
	string filenameIn = filename + '.' + ext, filenameOut = filename + outNameAppend + '.' + ext;
	string fileInName(path + filenameIn), fileOutName(path + filenameOut);

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
	return 0;
}