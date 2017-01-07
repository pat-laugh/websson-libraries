//MIT License
//Copyright(c) 2016 Patrick Laughrea
#include "RailroadDiagrams/railroadDiagrams.h"

#include <iostream>
#include <fstream>
#include <functional>

using namespace std;
using namespace webss;

const char ERROR_FAILED_OPEN_FILE[] = "Error: failed to open file ";
void readFiles()
{
	char c;
	vector<string> files
	{ {
		"binaryHead",
		"binaryList",
		"charValue",
		"colonValue",
		"colonValueFunctionHead",
//		"containerText",
//		"containerTextFunctionHead",
		"cstring",
		"dictionary",
		"document",
		"equalValue",
		"escapeSequence",
		"escapeSequenceUnicode",
		"function",
		"functionBody",
//		"functionBodyBinary",
//		"functionBodyMandatory",
//		"functionBodyStandard",
		"functionHead",
		"functionHeadBinary",
//		"functionHeadMandatory",
//		"functionHeadStandard",
		"import",
		"junk",
		"keyValue",
		"keyValueFunctionHead",
		"keyValueVariableKeyword",
		"keywordValue",
		"lineString",
		"list",
		"name",
		"nameBody",
		"nameStart",
		"number",
//		"numberInt",
//		"numberHex",
		"numberStart",
		"separator",
		"string",
		"stringBody",
		"textContainer",
		"textContainerFunctionHead",
		"textDictionary",
//		"textList",
//		"textTuple",
//		"textFunctionHead",
		"tuple",
		"typeKeyword",
		"usingNamespace",
		"value",
		"variableName",
		"variableInitialization",
		"variableKeywordInitialization",
		"variableString",
		"whitespace"
	} };

	auto parser = Deserializer::getParser();

	string path("C:\\Users\\Pat-Laugh\\Desktop\\WebSSON Doc\\Railroads\\");
	{
		string fileInName(path + "vars.txt");
		cout << endl << fileInName << endl;
		ifstream fileIn(fileInName, ios::binary);
		if (fileIn.fail()) { cerr << ERROR_FAILED_OPEN_FILE << '"' << fileInName << '"' << endl; cin >> c; return; }
		try { Webss data(parser.parse(fileIn)); }
		catch (exception e)
		{
			cerr << "Parse failed: " << e.what() << endl;
			cin >> c;
		}
		fileIn.close();
	}

	for (const auto& file : files)
	{
		string fileInName(path + file + ".txt"), fileOutName(path + file + "Out.txt");
		cout << endl << fileInName << endl;
		ifstream fileIn(fileInName, ios::binary);
		if (fileIn.fail()) { cerr << ERROR_FAILED_OPEN_FILE << '"' << fileInName << '"' << endl; cin >> c; return; }
		try
		{
			{
				string createFileIfNotExistName(path + file + "Svg.html");
				ifstream checkExist(createFileIfNotExistName, ios::in);
				if (checkExist.fail())
				{
					ofstream createFile(createFileIfNotExistName, ios::out);
					createFile << "<link href=\"railroads.css\" rel=\"stylesheet\">" << endl;
					createFile.close();
				}
			}

			ofstream fileOut(fileOutName, ios::binary);
			if (fileOut.fail()) { cerr << ERROR_FAILED_OPEN_FILE << '"' << fileOutName << '"' << endl; cin >> c; return; }
			
			try
			{
				fileOut << Deserializer::makeDiagram(parser, fileIn);
			}
			catch (exception e)
			{
				cerr << "Deserialization failed: " << e.what() << endl;
				cin >> c;
			}
			fileOut.close();
		}
		catch (exception e)
		{
			cerr << "Parse failed: " << e.what() << endl;
			cin >> c;
		}
		fileIn.close();
	}
	cin >> c;
}

int main(int argc, char** argv)
{
	if (argc == 1)
		readFiles();

	if (argc != 2)
	{
		cerr << "Usage: " << argv[0] << " file";
		return 1;
	}

	string fileInName(argv[1]), fileOutName;
	auto indexExt = fileInName.rfind('.');
	if (indexExt == string::npos)
		fileOutName = fileInName + "Out.txt";
	else
		fileOutName = fileInName.substr(0, indexExt) + "Out" + fileInName.substr(indexExt);

	ifstream fileIn(fileInName, ios::binary);
	if (fileIn.fail()) { cerr << ERROR_FAILED_OPEN_FILE << '"' << fileInName << '"' << endl; return 1; }
	try
	{
		ofstream fileOut(fileOutName, ios::binary);
		if (fileOut.fail()) { cerr << ERROR_FAILED_OPEN_FILE << '"' << fileOutName << '"' << endl; return 1; }
		try
		{
			fileOut << Deserializer::makeDiagram(fileIn);
		}
		catch (exception e)
		{
			cerr << "Deserialization failed: " << e.what() << endl;
		}
		fileOut.close();
	}
	catch (exception e)
	{
		cerr << "Parse failed: " << e.what() << endl;
	}
	fileIn.close();
	return 0;
}