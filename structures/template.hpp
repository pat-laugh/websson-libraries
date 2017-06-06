//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "base.hpp"
#include "templateBody.hpp"
#include "templateHead.hpp"

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
		This(Head head, Dictionary body) : Head(std::move(head)), Body(std::move(body)) {}
		This(Head head, List body) : Head(std::move(head)), Body(std::move(body)) {}
		This(Head head, Tuple body, WebssType type = WebssType::TUPLE) : Head(std::move(head)), Body(std::move(body), type) {}

		bool operator==(const This& o) const { return (this == &o) || (typename Head::operator==(o) && typename Body::operator==(o)); }
		bool operator!=(const This& o) const { return !(*this == o); }
	};
#undef This
}