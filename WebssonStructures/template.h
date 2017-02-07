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

		ClassName(TemplateHead&& Head, typename Body::Tuple&& Body, bool isText) : Head(std::move(Head)), Body(std::move(Body), true) {}
		ClassName(const TemplateHead& Head, const typename Body::Tuple& Body, bool isText) : Head(Head), Body(Body, true) {}
		ClassName(typename Head::Parameters&& Head, typename Body::Tuple&& Body, bool isText) : Head(std::move(Head)), Body(std::move(Body), true) {}
		ClassName(const typename Head::Parameters& Head, const typename Body::Tuple& Body, bool isText) : Head(Head), Body(Body, true) {}
		ClassName(const typename Head::Pointer& Head, typename Body::Tuple&& Body, bool isText) : Head(Head), Body(std::move(Body), true) {}
		ClassName(const typename Head::Pointer& Head, const typename Body::Tuple& Body, bool isText) : Head(Head), Body(Body, true) {}
		ClassName(const typename Head::Entity& Head, typename Body::Tuple&& Body, bool isText) : Head(Head), Body(std::move(Body), true) {}
		ClassName(const typename Head::Entity& Head, const typename Body::Tuple& Body, bool isText) : Head(Head), Body(Body, true) {}
	};
#undef ClassName
}