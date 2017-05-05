//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "base.h"
#include "templateHead.h"

namespace webss
{
	class TemplateScoped : public TemplateHeadScoped
	{
	private:
		Webss value;
	public:
		using Head = TemplateHeadScoped;

		TemplateScoped(Head&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		TemplateScoped(const Head& head, const Webss& value) : Head(head), value(value) {}

		const Webss& getValue() const { return value; }
	};
}