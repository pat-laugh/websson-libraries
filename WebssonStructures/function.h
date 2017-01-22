//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "functionHead.h"
#include "functionBody.h"

namespace webss
{
#define ClassName BasicFunction
	template <class FunctionHead, class Webss>
	class ClassName : public FunctionHead, public BasicFunctionBody<Webss>
	{
	private:
		using Head = FunctionHead;
		using Body = BasicFunctionBody<Webss>;
	public:
#define BodyParameter typename Body::Dictionary
#include "functionPattern.def"
#undef BodyParameter
#define BodyParameter typename Body::List
#include "functionPattern.def"
#undef BodyParameter
#define BodyParameter typename Body::Tuple
#include "functionPattern.def"
#undef BodyParameter
	};
#undef ClassName
}