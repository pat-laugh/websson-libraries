//MIT License
//Copyright 2017 Patrick Laughrea

#include <iostream>

#include "parser/parser.hpp"
#include "serializerHtml/serializerHtml.hpp"

using namespace std;
using namespace webss;

int main()
{
	cout << SerializerHtml::serialize(Parser(SmartIterator(cin.rdbuf())).parseDocument());
    return 0;
}

