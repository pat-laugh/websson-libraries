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
	public:
		DocumentHead& getHead() { return head; }
		const DocumentHead& getHead() const { return head; }
	private:
		DocumentHead head;
	};
}