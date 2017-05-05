//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "base.h"
#include "templateBody.h"
#include "templateHead.h"

namespace webss
{
#define This BasicTemplate
	template <class TemplateHead>
	class This : public TemplateHead, public TemplateBody
	{
	private:
		using Head = TemplateHead;
		using Body = TemplateBody;
	public:
		This(Head&& head, Dictionary&& body) : Head(std::move(head)), Body(std::move(body)) {}
		This(const Head& head, const Dictionary& body) : Head(head), Body(body) {}
		This(Head&& head, List&& body) : Head(std::move(head)), Body(std::move(body)) {}
		This(const Head& head, const List& body) : Head(head), Body(body) {}
		This(Head&& head, Tuple&& body) : Head(std::move(head)), Body(std::move(body)) {}
		This(const Head& head, const Tuple& body) : Head(head), Body(body) {}
		This(Head&& head, Tuple&& body, bool) : Head(std::move(head)), Body(std::move(body), true) {}
		This(const Head& head, const Tuple& body, bool) : Head(head), Body(body, true) {}
	};
#undef This
}