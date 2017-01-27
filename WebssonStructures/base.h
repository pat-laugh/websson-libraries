#pragma once

#include <vector>

namespace webss
{
	template <class Webss>
	class BasicBlockHead;

	template <class Webss>
	class BasicBlock;

	template <class Webss>
	class BasicDictionary;

	template <class Webss>
	class BasicDocument;

	template <class Webss>
	class BasicParamDocument;

//	template <class Webss>
//	using BasicDocumentHead = std::vector<BasicParamDocument<Webss>>;

	template <class Webss>
	class BasicScopedDocument;

	template <class Webss>
	class BasicImportedDocument;

	template <class Webss>
	class BasicParamDocument;

	template <class Webss>
	class BasicNamespace;

	template <class Webss>
	class BasicEntity;

	template <class Parameter>
	class BasicFunctionHead;

	template <class Webss>
	class BasicFunctionBody;

	template <class FunctionHead, class Webss>
	class BasicFunction;

//	template <class Webss>
//	class BasicParamScoped;

//	template <class Webss>
//	class BasicFunctionHeadScoped;

	template <class Webss>
	class BasicFunctionScoped;

	template <class Webss>
	class BasicList;

	template <class Webss>
	class BasicParamBinary;

	template <class Webss>
	class BasicParamStandard;

	template <class Webss>
	class BasicTuple;
}