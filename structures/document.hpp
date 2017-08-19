//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <vector>

#include "base.hpp"
#include "documentHead.hpp"
#include "tuple.hpp"

namespace webss
{
	class Document
	{
	public:
		enum class Alternate { HEAD, BODY, IMPORT };
		using AlternateVector = std::vector<Alternate>;
		
		DocumentHead& getHead();
		const DocumentHead& getHead() const;
		
		Tuple& getBody();
		const Tuple& getBody() const;
		
		const AlternateVector& getAlternate() const;
		
		bool operator==(const Document& o) const;
		bool operator!=(const Document& o) const;
		
		void addHead(ParamDocument param);
		
		void addBody(Webss value);
		void addBody(std::string key, Webss value);
		void addBodySafe(std::string key, Webss value);
		
		void addImport(Webss value);
		void addImportSafe(std::string key, Webss value);

	private:
		DocumentHead head;
		Tuple body;
		AlternateVector alternate;
	};
}