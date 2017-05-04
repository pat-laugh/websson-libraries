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
		This(Head&& head, typename Body::Dictionary&& body) : Head(std::move(head)), Body(std::move(body)) {}
		This(const Head& head, const typename Body::Dictionary& body) : Head(head), Body(body) {}
		This(Head&& head, typename Body::List&& body) : Head(std::move(head)), Body(std::move(body)) {}
		This(const Head& head, const typename Body::List& body) : Head(head), Body(body) {}
		This(Head&& head, typename Body::Tuple&& body) : Head(std::move(head)), Body(std::move(body)) {}
		This(const Head& head, const typename Body::Tuple& body) : Head(head), Body(body) {}
		This(Head&& head, typename Body::Tuple&& body, bool) : Head(std::move(head)), Body(std::move(body), true) {}
		This(const Head& head, const typename Body::Tuple& body, bool) : Head(head), Body(body, true) {}
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