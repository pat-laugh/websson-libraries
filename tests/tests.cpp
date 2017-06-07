#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "parser/parser.hpp"
#include "serializer/serializer.hpp"

using namespace std;
using namespace webss;

enum class ErrorType { NONE, FATAL, PARSE, SERIALIZE, PARSE_AFTER_SERIALIZATION, EQUALITY, TEST };

string makeCompleteFilenameIn(string filename);
string makeCompleteFilenameOut(string filename);
ErrorType test(string filename, function<void(const Document& doc)> checkResult);
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
		vector<string> filenames { "strings", "expandTuple", "templateBlock" };
		for (const auto& filename : filenames)
		{
			test(filename, [](const Document& doc) {});
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

//"SOFt assERT
void sofert(bool condition, string errorMsg = "")
{
	if (condition)
		return;
	throw exception(errorMsg == "" ? "assert failed" : errorMsg.c_str());
}

ErrorType tryParse(string filenameIn, Document& doc)
{
	ifstream fileIn(filenameIn, ios::binary);
	if (fileIn.fail())
	{
		cerr << "Error: failed to open file \"" << filenameIn << "\"" << endl;
		return ErrorType::FATAL;
	}

	try
	{
		doc = Parser(fileIn).parseDocument();
	}
	catch (const exception& e)
	{
		cerr << "Parse failed: " << e.what() << endl;
		return ErrorType::PARSE;
	}

	return ErrorType::NONE;
}

ErrorType trySerialize(string filenameOut, string& output, const Document& doc)
{
	ofstream fileOut(filenameOut, ios::binary);
	if (fileOut.fail())
	{
		cerr << "Error: failed to open file \"" << filenameOut << "\"" << endl;
		return ErrorType::FATAL;
	}

	try
	{
		output = Serializer::serialize(doc);
		fileOut << output;
		fileOut.flush();
	}
	catch (const exception& e)
	{
		cout << "Serialization failed: " << e.what() << endl;
		return ErrorType::SERIALIZE;
	}

	return ErrorType::NONE;
}

//you have to first verify that the parsed stuff is good
//then serialize it, parse the result, and make sure it
//equals the first parse
ErrorType test(string filename, function<void(const Document& doc)> checkResult)
{
	string filenameOut(makeCompleteFilenameOut(filename));

	cout << "Input: " << filename << endl;

	Document doc;
	ErrorType errorParse = tryParse(makeCompleteFilenameIn(filename), doc);
	if (errorParse != ErrorType::NONE)
		return errorParse;

	cout << "No errors while parsing" << endl;

	string output;
	ErrorType errorSerialize = trySerialize(makeCompleteFilenameOut(filename), output, doc);
	if (errorSerialize != ErrorType::NONE)
		return errorSerialize;

	cout << "No errors while serializing" << endl;

	Document newDoc;
	try
	{
		newDoc = Parser(output).parseDocument();
	}
	catch (const exception& e)
	{
		cout << "Parse after serialization failed: " << e.what() << endl;
		return ErrorType::PARSE_AFTER_SERIALIZATION;
	}

	if (newDoc != doc)
	{
		cout << "Equality failed: serialized document not equal to parsed doc" << endl;
		return ErrorType::EQUALITY;
	}

	try
	{
		checkResult(doc);
	}
	catch (const exception& e)
	{
		cout << "Test failed: " << e.what() << endl;
		return ErrorType::TEST;
	}

	cout << "No errors while testing" << endl;

	return ErrorType::NONE;
}

void testDictionary()
{
	test("dictionary", [](const Document& doc)
	{
		const auto& webssDict = doc.getData()[0];
		sofert(webssDict.isDictionary());
		const auto& dict = webssDict.getDictionary();
		sofert(dict.size() == 4);
		sofert(dict.has("a_list"));
		sofert(dict.has("key1"));
		sofert(dict.has("key2"));
		sofert(dict.has("other-dict"));
		sofert(dict["a_list"].isList() && dict["a_list"].getList().size() == 4);
		sofert((int)dict["a_list"][0] == 0 && (int)dict["a_list"][1] == 1 && (int)dict["a_list"][2] == 2 && (int)dict["a_list"][3] == 3);
		sofert(dict["key1"].isInt() && dict["key1"].getInt() == 2);
		sofert(dict["key2"].isString() && dict["key2"].getString() == "text");
		sofert(dict["other-dict"].isDictionary() && dict["other-dict"].getDictionary().has("key") && dict["other-dict"]["key"].isBool() && dict["other-dict"]["key"].getBool() == true);
	});
}