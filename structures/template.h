//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "base.h"
#include "templateBody.h"
#include "templateHead.h"
#include "templateScoped.h"

namespace webss
{
#define This BasicTemplate
	template <class TemplateHead, class Webss>
	class This : public TemplateHead, public BasicTemplateBody<Webss>
	{
	private:
		using Head = TemplateHead;
		using Body = BasicTemplateBody<Webss>;
	public:
		This(Head&& head, typename Body::Dictionary&& Body) : Head(std::move(head)), Body(std::move(Body)) {}
		This(const Head& head, const typename Body::Dictionary& Body) : Head(head), Body(Body) {}
		This(Head&& head, typename Body::List&& Body) : Head(std::move(head)), Body(std::move(Body)) {}
		This(const Head& head, const typename Body::List& Body) : Head(head), Body(Body) {}
		This(Head&& head, typename Body::Tuple&& Body) : Head(std::move(head)), Body(std::move(Body)) {}
		This(const Head& head, const typename Body::Tuple& Body) : Head(head), Body(Body) {}
		This(Head&& head, typename Body::Tuple&& Body, bool isText) : Head(std::move(head)), Body(std::move(Body), true) {}
		This(const Head& head, const typename Body::Tuple& Body, bool isText) : Head(head), Body(Body, true) {}
	};
#undef This

	#define This BasicTemplateScoped
	template <class Webss>
	class This : public BasicTemplateHeadScoped<Webss>
	{
	private:
		Webss value;
	public:
		using Head = BasicTemplateHeadScoped<Webss>;

		This(Head&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const Head& head, const Webss& value) : Head(head), value(value) {}

		const Webss& getValue() const { return value; }
	};
#undef This
}