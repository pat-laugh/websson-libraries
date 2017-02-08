//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "base.h"
#include "entity.h"
#include "templateHead.h"
#include "namespace.h"

namespace webss
{
	template <class Webss>
	using BasicParamScoped = BasicParamDocument<Webss>;

	template <class Webss>
	using BasicTemplateHeadScoped = BasicTemplateHead<BasicParamDocument<Webss>, Webss>;
}