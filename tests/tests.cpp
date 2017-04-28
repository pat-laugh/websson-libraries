#include <iostream>
#include <fstream>
#include <string>

#include "parser/parser.h"
#include "serializer/serializer.h"

using namespace std;
using namespace webss;

int main()
{
	char inChar;

	do
	{
		string filename("testWebsson");
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
			Parser parser(fileIn);
			data = parser.parseDocument();
			cout << "No errors while parsing" << endl;

			ofstream fileOut(fileOutName, ios::binary);
			if (fileOut.fail()) { cerr << "Error: failed to open file \"" << fileOutName << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
			try
			{
				fileOut << Serializer::serialize(data);
				fileOut.flush();
				cout << "No errors while deserializing" << endl;
			}
			catch (const exception& e)
			{
				cout << "Deserialization failed: " << e.what() << endl;
			}
			fileOut.close();
		}
		catch (const exception& e)
		{
			cout << "Parse failed: " << e.what() << endl;
		}
		fileIn.close();
		cin >> inChar;
	} while (inChar != 'q');

    return 0;
}

