#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#include "parser/parser.hpp"
#include "serializer/serializer.hpp"
#include "serializerHtml/serializerHtml.hpp"

using namespace std;
using namespace webss;

enum class ErrorType { NONE, FATAL, PARSE, SERIALIZE, PARSE_AFTER_SERIALIZATION, EQUALITY, TEST };

string makeCompleteFilenameIn(string filename);
string makeCompleteFilenameOut(string filename);
ErrorType test(string filename, function<void(const Document& doc)> checkResult);
void testDictionary();
void testTemplateStandard();
ErrorType testSerializerHtml();

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
		vector<string> filenames { "strings", "expandTuple", "templateBlock", "namespace", "enum",
			"list", "tuple", "names-keywords", "multiline-string-options",
			"option" };
		for (const auto& filename : filenames)
		{
			test(filename, [](const Document&) {});
			cout << endl;
		}
#ifdef TEST_PERFORMANCE
	}
#else
		testDictionary();
		cout << endl;
		testTemplateStandard();
		cout << endl;
		testSerializerHtml();
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

void softAssert(bool condition, unsigned int line)
{
	if (condition)
		return;
	throw logic_error(string("[ln ") + to_string(line) + "] assert failed");
}

//SOFt assERT
#define sofert(condition) { softAssert(condition, __LINE__); }

ErrorType tryParse(string filenameIn, Document& doc)
{
	ifstream fileIn(filenameIn, ios::binary);
	if (fileIn.fail())
	{
		cout << "Error: failed to open file \"" << filenameIn << "\"" << endl;
		return ErrorType::FATAL;
	}

	try
	{
		doc = Parser(fileIn).parseDocument();
	}
	catch (const exception& e)
	{
		cout << "Parse failed: " << e.what() << endl;
		return ErrorType::PARSE;
	}

	return ErrorType::NONE;
}

template <class Serializer>
ErrorType trySerialize(string filenameOut, string& output, const Document& doc)
{
	ofstream fileOut(filenameOut, ios::binary);
	if (fileOut.fail())
	{
		cout << "Error: failed to open file \"" << filenameOut << "\"" << endl;
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
	ErrorType errorSerialize = trySerialize<Serializer>(makeCompleteFilenameOut(filename), output, doc);
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

void testTemplateStandard()
{
	test("templateStandard", [](const Document& doc)
	{
		sofert(doc.size() == 5);
		sofert(doc.has("template1") && doc.has("template2"));
		const auto& templ1 = doc["template1"];
		const auto& templ2 = doc["template2"];
		sofert(templ1 == doc[0] && templ2 == doc[1]);
		sofert(templ1 == templ2);
		{
			sofert(templ1.isList());
			const auto& list = templ1.getList();
			sofert(list.size() == 2);
			sofert(list[0].isTuple());
			sofert(list[1].isTuple());
			{
				const auto& tuple = list[0].getTuple();
				sofert(tuple.size() == 3);
				sofert(tuple[0].isString() && tuple[0].getString() == "First");
				sofert(tuple[1].isString() && tuple[1].getString() == "Last");
				sofert(tuple[2].isInt() && tuple[2].getInt() == 38);
				sofert(tuple.has("firstName") && tuple.has("lastName") && tuple.has("age"));
				sofert(tuple["firstName"].isString() && tuple["firstName"].getString() == "First");
				sofert(tuple["lastName"].isString() && tuple["lastName"].getString() == "Last");
				sofert(tuple["age"].isInt() && tuple["age"].getInt() == 38);
			}
			{
				const auto& tuple = list[1].getTuple();
				sofert(tuple.size() == 3);
				sofert(tuple[0].isString() && tuple[0].getString() == "Second");
				sofert(tuple[1].isString() && tuple[1].getString() == "Third");
				sofert(tuple[2].isInt() && tuple[2].getInt() == 47);
				sofert(tuple.has("firstName") && tuple.has("lastName") && tuple.has("age"));
				sofert(tuple["firstName"].isString() && tuple["firstName"].getString() == "Second");
				sofert(tuple["lastName"].isString() && tuple["lastName"].getString() == "Third");
				sofert(tuple["age"].isInt() && tuple["age"].getInt() == 47);
			}
		}
		
		Tuple tupleTempl3;
		const auto& templ3 = doc[2];
		{
			sofert(templ3.isTuple());
			const auto& tuple = templ3.getTuple();
			sofert(tuple.size() == 2);
			sofert(tuple[0].isString() && tuple[0].getString() == "default1");
			sofert(tuple[1].isString() && tuple[1].getString() == "default2");
			sofert(tuple.has("val1") && tuple.has("val2"));
			sofert(tuple["val1"].isString() && tuple["val1"].getString() == "default1");
			sofert(tuple["val2"].isString() && tuple["val2"].getString() == "default2");
			tupleTempl3 = tuple;
		}
		
		const auto& templ4 = doc[3];
		sofert(templ4.isList() && templ4.getList().empty());
		
		const auto& templ5 = doc[4];
		{
			sofert(templ5.isList());
			const auto& list = templ5.getList();
			sofert(list.size() == 1);
			sofert(list[0].isTuple());
			sofert(tupleTempl3 == list[0].getTuple());
		}
	});
}

ErrorType testSerializerHtml()
{
	string filename("test-serializer-html");
	string filenameIn("files-serializer-html/" + filename + ".wbsn");
	string filenameOut(filename + ".wbsnout"); //not html as not yet in .gitignore

	cout << "Input: " << filename << endl;

	Document doc;
	ErrorType errorParse = tryParse(filenameIn, doc);
	if (errorParse != ErrorType::NONE)
		return errorParse;

	cout << "No errors while parsing" << endl;

	string output;
	ErrorType errorSerialize = trySerialize<SerializerHtml>(filenameOut, output, doc);
	if (errorSerialize != ErrorType::NONE)
		return errorSerialize;

	cout << "No errors while serializing" << endl;

	return ErrorType::NONE;
}