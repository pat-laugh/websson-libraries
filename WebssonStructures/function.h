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
		using head = FunctionHead;
		using body = BasicFunctionBody<Webss>;
	public:
#define BodyParameter typename body::Dictionary
#include "functionPattern.def"
#undef BodyParameter
#define BodyParameter typename body::List
#include "functionPattern.def"
#undef BodyParameter
#define BodyParameter typename body::Tuple
#include "functionPattern.def"
#undef BodyParameter
	};
#undef ClassName
}