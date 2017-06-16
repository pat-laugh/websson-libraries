//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "base.hpp"
#include "templateBody.hpp"
#include "templateHead.hpp"

namespace webss
{
#define This BasicTemplate
	template <class Thead>
	class This : public Thead, public TemplateBody
	{
	private:
		using Head = Thead;
		using Body = TemplateBody;

	public:
		This(Head head, List body) : Head(std::move(head)), Body(std::move(body)) {}
		This(Head head, Tuple body, WebssType type = WebssType::TUPLE) : Head(std::move(head)), Body(std::move(body), type) {}

		bool operator==(const This& o) const { return (this == &o) || (Head::operator==(o) && Body::operator==(o)); }
		bool operator!=(const This& o) const { return !(*this == o); }
	};
#undef This
}