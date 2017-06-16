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
	class TemplateBody;
	class ParamBin;
	class ParamStd;
	class Document;

	template <class Param>
	class BasicThead;

	template <class Thead>
	class BasicTemplate;

	template <class Thead>
	class BasicTemplatePlus;

	class ParamDocument;
	using DocumentHead = std::vector<ParamDocument>;

	class TheadSelf {}; //empty class

	using TheadBin = BasicThead<ParamBin>;
	using TheadStd = BasicThead<ParamStd>;
	using TemplateBin = BasicTemplate<TheadBin>;
	using TemplateStd = BasicTemplate<TheadStd>;
	using TemplatePlusBin = BasicTemplatePlus<TheadBin>;
	using TemplatePlusStd = BasicTemplatePlus<TheadStd>;
}