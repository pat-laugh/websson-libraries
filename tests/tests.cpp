#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "parser/parser.hpp"
#include "serializer/serializer.hpp"

using namespace std;
using namespace webss;

string makeCompleteFilenameIn(string filename);
string makeCompleteFilenameOut(string filename);
void testDictionary();

char inChar;

int main()
{
//#define TEST_PERFORMANCE
#ifdef TEST_PERFORMANCE
	for (int i = 0; i < 2000; ++i)
#else
	do
#endif
	{
		vector<string> filenames { "strings", "expandTuple" };
		for (const auto& filename : filenames)
		{
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
					ifstream fileExpected(filenameExpected, ios::in);
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
		}
#ifdef TEST_PERFORMANCE
	}
#else
		testDictionary();
		cin >> inChar;
	} while (inChar != 'q');
#endif

    return 0;
}

string makeCompleteFilenameIn(string filename)
{
	return "files-in/" + filename + ".wbsn";
}
string makeCompleteFilenameOut(string filename)
{
	return filename + ".wbsnout";
}

void testDictionary()
{
	string filename("dictionary");
	Document data;
	string filenameIn(makeCompleteFilenameIn(filename));
	string filenameOut(makeCompleteFilenameOut(filename));

	cout << "Input: " << filenameIn << endl;

	ifstream fileIn(filenameIn, ios::binary);
	if (fileIn.fail()) { cerr << "Error: failed to open file \"" << filenameIn << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
	try
	{
		Parser parser(fileIn);
		data = parser.parseDocument();

		const auto& webssDict = data.getData()[0];
		assert(webssDict.isDictionary());
		const auto& dict = webssDict.getDictionary();
		assert(dict.size() == 4);
		assert(dict.has("a_list"));
		assert(dict.has("key1"));
		assert(dict.has("key2"));
		assert(dict.has("other-dict"));
		assert(dict["a_list"].isList() && dict["a_list"].getList().size() == 4);
		assert((int)dict["a_list"][0] == 0 && (int)dict["a_list"][1] == 1 && (int)dict["a_list"][2] == 2 && (int)dict["a_list"][3] == 3);
		assert(dict["key1"].isInt() && dict["key1"].getInt() == 2);
		assert(dict["key2"].isString() && dict["key2"].getString() == "text");
		assert(dict["other-dict"].isDictionary() && dict["other-dict"].getDictionary().has("key") && dict["other-dict"]["key"].isBool() && dict["other-dict"]["key"].getBool() == true);

		cout << "No errors while parsing" << endl;

		ofstream fileOut(filenameOut, ios::binary);
		if (fileOut.fail()) { cerr << "Error: failed to open file \"" << filenameOut << "\"" << endl; cin >> inChar; exit(EXIT_FAILURE); }
		try
		{
			auto output = Serializer::serialize(data);
			fileOut << output;
			fileOut.flush();
			cout << "No errors while serializing" << endl;
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
}