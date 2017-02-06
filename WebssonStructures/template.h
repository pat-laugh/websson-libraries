//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "base.h"
#include "templateHead.h"
#include "templateBody.h"

namespace webss
{
#define ClassName BasicTemplate
	template <class TemplateHead, class Webss>
	class ClassName : public TemplateHead, public BasicTemplateBody<Webss>
	{
	private:
		using Head = TemplateHead;
		using Body = BasicTemplateBody<Webss>;
	public:
#define BodyParameter typename Body::Dictionary
#include "templatePattern.def"
#undef BodyParameter
#define BodyParameter typename Body::List
#include "templatePattern.def"
#undef BodyParameter
#define BodyParameter typename Body::Tuple
#include "templatePattern.def"
#undef BodyParameter
	};
#undef ClassName
}