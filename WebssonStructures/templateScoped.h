//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "base.h"
#include "entity.h"
#include "namespace.h"
#include "templateHead.h"

namespace webss
{
	template <class Webss>
	using BasicParamScoped = BasicParamDocument<Webss>;

	template <class Webss>
	using BasicTemplateHeadScoped = BasicTemplateHead<BasicParamDocument<Webss>, Webss>;
}