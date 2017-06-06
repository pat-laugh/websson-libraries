//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include "base.hpp"
#include "documentHead.hpp"
#include "sharedMap.hpp"

namespace webss
{
	class Document : public BasicSharedMap<Webss>
	{
	private:
		using base = BasicSharedMap<Webss>;
	public:
		DocumentHead& getHead() { return head; }
		const DocumentHead& getHead() const { return head; }
		
		bool operator==(const Document& o) const
		{
			return (this == &o) || (keys == o.keys && data == o.data && head == o.head);
		}
		bool operator!=(const Document& o) const
		{
			return !(*this == o);
		}

	private:
		DocumentHead head;
	};
}