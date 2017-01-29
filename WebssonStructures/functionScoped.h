#pragma once

#include "base.h"
#include "entity.h"
#include "functionHead.h"
#include "namespace.h"

namespace webss
{
	template <class Webss>
	using BasicParamScoped = BasicParamDocument<Webss>;

	template <class Webss>
	using BasicFunctionHeadScoped = BasicFunctionHead<BasicParamDocument<Webss>>;

#define This BasicFunctionScoped
	template <class Webss>
	class This : public BasicFunctionHeadScoped<Webss>
	{
	private:
		Webss value;
	public:
		using Head = BasicFunctionHeadScoped<Webss>;
		using HeadParameters = typename Head::Parameters;
		using HeadPointer = typename Head::Pointer;
		using HeadEntity = typename Head::Entity;

		This(Head&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const Head& head, const Webss& value) : Head(head), value(value) {}
		This(HeadParameters&& head, Webss&& value) : Head(std::move(head)), value(std::move(value)) {}
		This(const HeadParameters& head, const Webss& value) : Head(head), value(value) {}
		This(const HeadPointer& head, Webss&& value) : Head(head), value(std::move(value)) {}
		This(const HeadPointer& head, const Webss& value) : Head(head), value(value) {}
		This(const HeadEntity& head, Webss&& value) : Head(head), value(std::move(value)) {}
		This(const HeadEntity& head, const Webss& value) : Head(head), value(value) {}

		const Webss& getValue() const { return value; }
	};
#undef This
}