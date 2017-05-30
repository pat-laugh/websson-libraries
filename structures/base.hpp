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
	class BlockHead;
	class Block;
	class TemplateBody;
	class ParamBinary;
	class ParamStandard;
	class Document;

	template <class Param>
	class BasicTemplateHead;

	template <class TemplateHead>
	class BasicTemplate;

	class ParamDocument;
	using DocumentHead = std::vector<ParamDocument>;

	class TemplateHeadSelf {}; //empty class

	using TemplateHeadBinary = BasicTemplateHead<ParamBinary>;
	using TemplateHeadStandard = BasicTemplateHead<ParamStandard>;
	using TemplateBinary = BasicTemplate<TemplateHeadBinary>;
	using TemplateStandard = BasicTemplate<TemplateHeadStandard>;
	
}