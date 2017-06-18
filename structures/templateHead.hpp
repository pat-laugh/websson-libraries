//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "parameters.hpp"
#include "webss.hpp"

namespace webss
{
	template <class Param>
	class BasicThead
	{
	public:
		using Params = BasicParams<Param>;
		using size_type = typename Params::size_type;

		BasicThead() {}
		BasicThead(Params params) : params(std::move(params)) {}

		bool operator==(const BasicThead& o) const { return params == o.params; }
		bool operator!=(const BasicThead& o) const { return !(*this == o); }

		bool empty() const { return params.empty(); }
		size_type size() const { return params.size(); }

		Param& back() { return params.back(); }
		const Param& back() const { return params.back(); }
		Param& last() { return back(); }
		const Param& last() const { return back(); }

		void attach(Param value) { params.add(std::move(value)); }

		void attachEmpty(std::string key) { attach(std::move(key), Param()); }

		void attach(std::string key, Param value) { params.addSafe(std::move(key), std::move(value)); }

		void attach(const BasicThead& value)
		{
			if (value.empty())
				return;

			const auto& valueTuple = value.params;
			if (params.empty())
				params = valueTuple.makeCompleteCopy();
			else
				params.merge(valueTuple);
		}

		const Params& getParams() const { return params; }

	private:
		Params params;
	};
}