#include <iostream>
#include <fstream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#endif


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
void testTemplateStd();
ErrorType testSerializerHtml();
template <class Element>
bool hasKeys(const Element& elem, set<string> keys);
template <>
bool hasKeys<Document>(const Document& doc, set<string> keys);

int main()
{
	vector<string> filenames
	{
		"strings", "namespace", "enum", "names-keywords", "multiline-string-options", "assignAbstractEntity",
		"expandTuple", "templatePlus", "list", "tuple", "option", "expandThead", "import", "templateFunction",
		"templateForeach",
	};
	for (const auto& filename : filenames)
	{
		test(filename, [](const Document&) {});
		cout << endl;
	}

	testDictionary();
	cout << endl;
	testTemplateStd();
	cout << endl;
	testSerializerHtml();

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

void softAssertBool(const Webss& value, bool b, unsigned int line)
{
	softAssert(value.isBool() && value.getBool() == b, line);
}

void softAssertInt(const Webss& value, WebssInt i, unsigned int line)
{
	softAssert(value.isInt() && value.getInt() == i, line);
}

void softAssertDouble(const Webss& value, double d, unsigned int line)
{
	softAssert(value.isDouble() && value.getDouble() == d, line);
}

void softAssertString(const Webss& value, string s, unsigned int line)
{
	softAssert(value.isString() && value.getString() == s, line);
}

//SOFt assERT
#define sofert(condition) softAssert(condition, __LINE__)
#define sofertBool(value, b) softAssertBool(value, b, __LINE__)
#define sofertInt(value, i) softAssertInt(value, i, __LINE__)
#define sofertDouble(value, d) softAssertDouble(value, d, __LINE__)
#define sofertString(value, s) softAssertString(value, s, __LINE__)

#ifdef _WIN32
int getConsoleColor() {
	CONSOLE_SCREEN_BUFFER_INFO info;
	if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
		return 7; //gray
	return info.wAttributes;
}
const auto DEFAULT_CONSOLE_COLOR = getConsoleColor();
#endif

void cStop()
{
#ifndef _WIN32
	cout << "\x1b[0m";
#else
	SetConsoleTextAttribute(hConsole, DEFAULT_CONSOLE_COLOR);
#endif
}

void cRed()
{
#ifndef _WIN32
	cout << "\x1b[31;1m";
#else
	SetConsoleTextAttribute(hConsole, 12);  //4: dark red, 12: red
#endif
}

void cGreen()
{
#ifndef _WIN32
	cout << "\x1b[32;1m";
#else
	SetConsoleTextAttribute(hConsole, 2); //2: green, 10: light green
#endif
}

void putRed(string content)
{
	cRed();
	cout << content;
	cStop();
}

void putGreen(string content)
{
	cGreen();
	cout << content;
	cStop();
}

ErrorType tryParse(string filenameIn, Document& doc)
{
	ifstream fileIn(filenameIn, ios::binary);
	if (fileIn.fail())
	{
		cout << endl;
		putRed("Error");
		cout << ": failed to open file \"" << filenameIn << "\"" << endl;
		return ErrorType::FATAL;
	}

	try
	{
		doc = Parser(fileIn, filenameIn).parseDocument();
	}
	catch (const exception& e)
	{
		cout << endl;
		putRed("Parse failed");
		cout << ": " << e.what() << endl;
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
		cout << endl;
		putRed("Error");
		cout << ": failed to open file \"" << filenameOut << "\"" << endl;
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
		cout << endl;
		putRed("Serialization failed");
		cout << ": " << e.what() << endl;
		return ErrorType::SERIALIZE;
	}

	return ErrorType::NONE;
}

//you have to first verify that the parsed stuff is good
//then serialize it, parse the result, and make sure it
//equals the first parse
ErrorType test(string filename, function<void(const Document& doc)> checkResult)
{
	auto filenameIn = makeCompleteFilenameIn(filename);
	auto filenameOut = makeCompleteFilenameOut(filename);

	cout << "Input: " << filename;

	Document doc;
	ErrorType errorParse = tryParse(filenameIn, doc);
	if (errorParse != ErrorType::NONE)
		return errorParse;

	cout << endl << "No errors while ";
	putGreen("parsing");

	string output;
	ErrorType errorSerialize = trySerialize<Serializer>(filenameOut, output, doc);
	if (errorSerialize != ErrorType::NONE)
		return errorSerialize;

	cout << ", ";
	putGreen("serializing");

	Document newDoc;
	try
	{
		newDoc = Parser(output, filenameIn).parseDocument();
	}
	catch (const exception& e)
	{
		cout << endl;
		putRed("Parse after serialization failed");
		cout << ": " << e.what() << endl;
		return ErrorType::PARSE_AFTER_SERIALIZATION;
	}

	if (newDoc != doc)
	{
		cout << endl;
		putRed("Equality failed");
		cout << ": serialized document not equal to parsed doc" << endl;
		return ErrorType::EQUALITY;
	}

	try
	{
		checkResult(doc);
	}
	catch (const exception& e)
	{
		cout << endl;
		putRed("Test failed");
		cout << ": " << e.what() << endl;
		return ErrorType::TEST;
	}

	cout << ", and ";
	putGreen("testing");
	cout << endl;

	return ErrorType::NONE;
}

void testDictionary()
{
	test("dictionary", [](const Document& doc)
	{
		const auto& webssDict = doc.getBody()[0];
		sofert(webssDict.isDictionary());
		const auto& dict = webssDict.getDictionary();
		sofert(dict.size() == 4);
		sofert(hasKeys(dict, { "a_list", "key1", "key2", "other-dict" }));
		sofert(dict["a_list"].isList() && dict["a_list"].getList().size() == 4);
		sofert((int)dict["a_list"][0] == 0 && (int)dict["a_list"][1] == 1 && (int)dict["a_list"][2] == 2 && (int)dict["a_list"][3] == 3);
		sofertInt(dict["key1"], 2);
		sofertString(dict["key2"], "text");
		sofert(dict["other-dict"].isDictionary() && dict["other-dict"].getDictionary().has("key") && dict["other-dict"]["key"].isBool() && dict["other-dict"]["key"].getBool() == true);
	});
}

void testTemplateStd()
{
	test("templateStandard", [](const Document& doc)
	{
		sofert(doc.getBody().size() == 5);
		sofert(hasKeys(doc, { "template1", "template2" }));
		const auto& templ1 = doc.getBody()["template1"];
		const auto& templ2 = doc.getBody()["template2"];
		sofert(templ1 == doc.getBody()[0] && templ2 == doc.getBody()[1]);
		sofert(templ1 == templ2);
		{
			sofert(templ1.isList());
			const auto& list = templ1.getList();
			sofert(list.size() == 2);
			sofert(list[0].isTemplate());
			sofert(list[1].isTemplate());
			{
				const auto& tuple = list[0].getTemplate().body.getTuple();
				sofert(tuple.size() == 3);
				sofertString(tuple[0], "First");
				sofertString(tuple[1], "Last");
				sofertInt(tuple[2], 38);
				sofert(hasKeys(tuple, { "firstName", "lastName", "age" }));
				sofertString(tuple["firstName"], "First");
				sofertString(tuple["lastName"], "Last");
				sofertInt(tuple["age"], 38);
			}
			{
				const auto& tuple = list[1].getTemplate().body.getTuple();
				sofert(tuple.size() == 3);
				sofertString(tuple[0], "Second");
				sofertString(tuple[1], "Third");
				sofertInt(tuple[2], 47);
				sofert(hasKeys(tuple, { "firstName", "lastName", "age" }));
				sofertString(tuple["firstName"], "Second");
				sofertString(tuple["lastName"], "Third");
				sofertInt(tuple["age"], 47);
			}
		}

		Tuple tupleTempl3;
		const auto& templ3 = doc.getBody()[2];
		{
			sofert(templ3.isTemplate());
			const auto& tuple = templ3.getTemplate().body.getTuple();
			sofert(tuple.size() == 2);
			sofertString(tuple[0], "default1");
			sofertString(tuple[1], "default2");
			sofert(hasKeys(tuple, { "val1", "val2" }));
			sofertString(tuple["val1"], "default1");
			sofertString(tuple["val2"], "default2");
			tupleTempl3 = tuple;
		}

		const auto& templ4 = doc.getBody()[3];
		sofert(templ4.isList() && templ4.getList().empty());

		const auto& templ5 = doc.getBody()[4];
		{
			sofert(templ5.isList());
			const auto& list = templ5.getList();
			sofert(list.size() == 1);
			sofert(list[0].isTemplate());
			sofert(tupleTempl3 == list[0].getTemplate().body.getTuple());
		}
	});
}

ErrorType testSerializerHtml()
{
	string filename("test-serializer-html");
	string filenameIn("files-serializer-html/" + filename + ".wbsn");
	string filenameOut(filename + ".wbsnout"); //not html as not yet in .gitignore

	cout << "Input: " << filename;

	Document doc;
	ErrorType errorParse = tryParse(filenameIn, doc);
	if (errorParse != ErrorType::NONE)
		return errorParse;

	cout << endl << "No errors while ";
	putGreen("parsing");

	string output;
	ErrorType errorSerialize = trySerialize<SerializerHtml>(filenameOut, output, doc);
	if (errorSerialize != ErrorType::NONE)
		return errorSerialize;

	cout << " and ";
	putGreen("serializing");
	cout << endl;

	return ErrorType::NONE;
}

template <class Element>
bool hasKeys(const Element& elem, set<string> keys)
{
	for (const auto& key : keys)
		if (!elem.has(key))
			return false;
	return true;
}

template <>
bool hasKeys<Document>(const Document& doc, set<string> keys)
{
	for (const auto& key : keys)
		if (!doc.getBody().has(key))
			return false;
	return true;
}