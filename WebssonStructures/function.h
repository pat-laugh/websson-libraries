//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "functionHead.h"
#include "functionBody.h"

namespace webss
{
#define ClassName BasicFunctionSingle
	template <class FunctionHead, class Webss>
	class ClassName : public FunctionHead, public BasicFunctionBodySingle<Webss>
	{
	private:
		using head = FunctionHead;
		using body = BasicFunctionBodySingle<Webss>;
	public:
#define BodyParameter typename body::Tuple
#include "functionPattern.def"
#undef BodyParameter

		~ClassName() {}

		/*
		BasicFunctionSingle(BasicFunctionSingle&& o) : head(std::move(o)), body(std::move(o)) {}
		BasicFunctionSingle(const BasicFunctionSingle& o) : head(o), body(o) {}

		BasicFunctionSingle& operator=(BasicFunctionSingle&& o)
		{
			if (this != &o)
			{
				head::operator=(std::move(o);
				body::operator=(std::move(o);
			}
			return *this;
		}
		BasicFunctionSingle& operator=(const BasicFunctionSingle& o)
		{
			if (this != &o)
			{
				head::operator=(o);
				body::operator=(o);
			}
			return *this;
		}*/
	};
#undef ClassName

#define ClassName BasicFunctionDual
	template <class FunctionHead, class Webss>
	class ClassName : public FunctionHead, public BasicFunctionBodyDual<Webss>
	{
	private:
		using head = FunctionHead;
		using body = BasicFunctionBodyDual<Webss>;
	public:
#define BodyParameter typename body::List
#include "functionPattern.def"
#undef BodyParameter
#define BodyParameter typename body::Tuple
#include "functionPattern.def"
#undef BodyParameter

		~ClassName() {}
		/*
		BasicFunctionDual(BasicFunctionDual&& o) : head(std::move(o)), body(std::move(o)) {}
		BasicFunctionDual(const BasicFunctionDual& o) : head(o), body(o) {}

		BasicFunctionDual& operator=(BasicFunctionDual&& o)
		{
			if (this != &o)
			{
				head::operator=(std::move(o);
				body::operator=(std::move(o);
			}
			return *this;
		}
		BasicFunctionDual& operator=(const BasicFunctionDual& o)
		{
			if (this != &o)
			{
				head::operator=(o);
				body::operator=(o);
			}
			return *this;
		}*/
	};
#undef ClassName
}