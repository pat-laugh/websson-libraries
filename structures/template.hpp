//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "base.hpp"
#include "thead.hpp"

namespace webss
{
	class Template : public Thead
	{
	private:
		using base = Thead;

	public:
		Webss body, content;

		Template(base thead, Tuple body, WebssType type = WebssType::TUPLE) : base(std::move(thead)), body(std::move(body), type) {}
		Template(base thead, Tuple body, Webss content, WebssType type = WebssType::TUPLE) : base(std::move(thead)), body(std::move(body), type), content(std::move(content)) {}

		bool operator==(const Template& o) const { return (this == &o) || (base::operator==(o) && body == o.body && content == o.content); }
		bool operator!=(const Template& o) const { return !(*this == o); }
	};
}