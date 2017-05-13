//MIT License
//Copyright 2017 Patrick Laughrea
#include "containerSwitcher.hpp"

using namespace std;
using namespace webss;

Parser::ContainerSwitcher::ContainerSwitcher(Parser& parser, ConType newCon, bool newAllowVoid)
	: parser(parser), oldCon(parser.con), oldAllowVoid(parser.allowVoid), oldMultilineContainer(parser.multilineContainer)
{
	parser.con = newCon;
	parser.allowVoid = newAllowVoid;
	parser.multilineContainer = checkLineEmpty(++parser.it);
}

Parser::ContainerSwitcher::~ContainerSwitcher()
{
	parser.con = oldCon;
	parser.allowVoid = oldAllowVoid;
	parser.multilineContainer = oldMultilineContainer;
}