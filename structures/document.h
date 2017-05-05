//MIT License
//Copyright(c) 2017 Patrick Laughrea
#pragma once

#include "base.h"
#include "documentHead.h"
#include "sharedMap.h"

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