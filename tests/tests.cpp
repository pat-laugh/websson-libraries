#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "parser/parser.hpp"
#include "serializer/serializer.hpp"

using namespace std;
using namespace webss;

int main()
{
	char inChar;

//#define TEST_PERFORMANCE
#ifdef TEST_PERFORMANCE
	for (int i = 0; i < 2000; ++i)
#else
	do
#endif
	{
		string filename("strings");
		Document data;
		string filenameIn("files-in/" + filename + ".wbsn");
		string filenameOut(filename + ".wbsnout");
		string filenameExpected("files-expected/" + filename + ".wbsn");

		cout << "Input: " << filenameIn << endl;

		ifstream fileIn(filenameIn, ios::binary);
		if (fileIn.fail()) { cerr << "Error: failed to open file \"" << filenameIn << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
		try
		{
			Parser parser(fileIn);
			data = parser.parseDocument();
			cout << "No errors while parsing" << endl;

			ofstream fileOut(filenameOut, ios::binary);
			if (fileOut.fail()) { cerr << "Error: failed to open file \"" << filenameOut << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
			try
			{
				auto output = Serializer::serialize(data);
				fileOut << output;
				fileOut.flush();
				ifstream fileExpected(filenameExpected, ios::binary);
				if (fileExpected.fail()) { cerr << "Error: failed to open file \"" << filenameExpected << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
				stringstream ss;
				ss << fileExpected.rdbuf();
				if (ss.str() != output)
					cout << "Serialization is not as expected" << endl;
				else
					cout << "No errors while serializing" << endl;
				fileExpected.close();
			}
			catch (const exception& e)
			{
				cout << "Serialization failed: " << e.what() << endl;
			}
			fileOut.close();
		}
		catch (const exception& e)
		{
			cout << "Parse failed: " << e.what() << endl;
		}
		fileIn.close();
#ifdef TEST_PERFORMANCE
	}
#else
		cin >> inChar;
	} while (inChar != 'q');
#endif

    return 0;
}
