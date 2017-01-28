//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "base.h"
#include "documentHead.h"
#include "sharedMap.h"

namespace webss
{
	template <class Webss>
	class BasicDocument : public BasicSharedMap<Webss>
	{
	public:
		using Head = BasicDocumentHead<Webss>;
		Head& getHead() { return head; }
		const Head& getHead() const { return head; }
	private:
		Head head;
	};
}