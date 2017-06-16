//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "template.hpp"

namespace webss
{
#define This BasicTemplatePlus
	template <class TemplateHead>
	class This : public BasicTemplate<TemplateHead>
	{
	private:
		using base = BasicTemplate<TemplateHead>;
		Webss content;

	public:
		This(base templ, Webss content) : base(move(templ)), content(std::move(content)) {}

		bool operator==(const This& o) const { return (this == &o) || (base::operator==(o) && content == o.content); }
		bool operator!=(const This& o) const { return !(*this == o); }

		const Webss& getContent() const { return content; }
	};
#undef This
}