//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <vector>

namespace webss
{
	class Webss;
	class Dictionary;
	class List;
	class Tuple;
	class Entity;
	class Namespace;
	class ParamBin;
	class ParamStd;
	class Document;

	template <class Param>
	class BasicThead;

	class Thead;
	class Template;

	class ParamDocument;
	using DocumentHead = std::vector<ParamDocument>;

	class TheadSelf {}; //empty class

	using TheadBin = BasicThead<ParamBin>;
	using TheadStd = BasicThead<ParamStd>;
	class TheadFun;
}