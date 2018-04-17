//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <string>

#include "base.hpp"
#include "enum.hpp"
#include "entity.hpp"
#include "keywords.hpp"
#include "namespace.hpp"
#include "stringList.hpp"
#include "thead.hpp"
#include "types.hpp"
#include "typeWebss.hpp"

namespace webss
{
	using Default = std::shared_ptr<Webss>;

	class Webss
	{
	public:
		Webss();
		~Webss();

		Webss(Webss&& o);
		Webss(const Webss& o);

		Webss& operator=(Webss o);

		Webss(const Entity& ent);
		Webss(const Default& tDefault);
		Webss(const Namespace& nspace);
		Webss(const Enum& tEnum);

		explicit Webss(bool b);
		Webss(Keyword keyword);
		Webss(int i);
		Webss(WebssInt i);
		Webss(size_t i);
		Webss(double d);

		Webss(const char* s);
		Webss(std::string s);
		Webss(StringList s);

		Webss(Document document);
		Webss(Dictionary dict);
		Webss(List list, WebssType type = WebssType::LIST);
		Webss(Tuple tuple, WebssType type = WebssType::TUPLE);
		Webss(Thead thead);
		Webss(Template templ);
		Webss(Placeholder placeholder);

		Webss(void* ptr, WebssType type);

		bool operator==(const Webss& o) const;
		bool operator!=(const Webss& o) const;

		const Webss& operator[](int index) const;
		const Webss& operator[](const std::string& key) const;
		const Webss& at(int index) const;
		const Webss& at(const std::string& key) const;

		const Webss& getWebssLast() const;

		WebssType getType() const;
		bool getBool() const;
		WebssInt getInt() const;
		double getDouble() const;
		const std::string& getString() const;
		const StringList& getStringList() const;
		const Document& getDocument() const;
		const Dictionary& getDictionary() const;
		const List& getList() const;
		const Tuple& getTuple() const;
		const Thead& getThead() const;
		const Template& getTemplate() const;
		const Namespace& getNamespace() const;
		const Enum& getEnum() const;

		WebssType getTypeRaw() const;

		const Entity& getEntityRaw() const;
		const Default& getDefaultRaw() const;
		const Namespace& getNamespaceRaw() const;
		const Enum& getEnumRaw() const;
		const Placeholder& getPlaceholderRaw() const;

		bool getBoolRaw() const;
		WebssInt getIntRaw() const;
		double getDoubleRaw() const;
		const std::string& getStringRaw() const;
		const StringList& getStringListRaw() const;
		const Document& getDocumentRaw() const;
		const Dictionary& getDictionaryRaw() const;
		const List& getListRaw() const;
		const Tuple& getTupleRaw() const;
		const Thead& getTheadRaw() const;
		const Template& getTemplateRaw() const;

		Entity& getEntityRaw();
		Default& getDefaultRaw();
		Namespace& getNamespaceRaw();
		Enum& getEnumRaw();
		Placeholder& getPlaceholderRaw();

		std::string& getStringRaw();
		StringList& getStringListRaw();
		Document& getDocumentRaw();
		Dictionary& getDictionaryRaw();
		List& getListRaw();
		Tuple& getTupleRaw();
		Thead& getTheadRaw();
		Template& getTemplateRaw();

		explicit operator bool() const { return getBool(); }
		explicit operator int() const { return (int)getInt(); }
		explicit operator long() const { return (long)getInt(); }
		explicit operator WebssInt() const { return getInt(); }
		explicit operator float() const { return (float)getDouble(); }
		explicit operator double() const { return getDouble(); }
		explicit operator const std::string&() const { return getString(); }
		explicit operator const StringList&() const { return getStringList(); }
		explicit operator const Document&() const { return getDocument(); }
		explicit operator const Dictionary&() const { return getDictionary(); }
		explicit operator const List&() const { return getList(); }
		explicit operator const Tuple&() const { return getTuple(); }
		explicit operator const Namespace&() const { return getNamespace(); }
		explicit operator const Enum&() const { return getEnum(); }

		template <class Element>
		const Element& getElement() const { return static_cast<const Element&>(*this); }

		bool isNone() const;
		bool isNull() const;
		bool isBool() const;
		bool isInt() const;
		bool isDouble() const;
		bool isString() const;
		bool isStringList() const;
		bool isDocument() const;
		bool isDictionary() const;
		bool isList() const;
		bool isTuple() const;
		bool isListText() const;
		bool isTupleText() const;
		bool isThead() const;
		bool isTemplate() const;
		bool isNamespace() const;
		bool isEnum() const;

		bool isAbstract() const;
		bool isConcrete() const;

	private:
		WebssType type = WebssType::NONE;
		union
		{
			bool tBool;
			WebssInt tInt;
			double tDouble;
			std::string* tString;
			StringList* stringList;
			Document* document;
			Dictionary* dict;
			List* list;
			Tuple* tuple;
			Thead* thead;
			Template* templ;
			Namespace nspace;
			Enum tEnum;
			Entity ent;
			Default tDefault;
			Placeholder* placeholder;
			void* ptr;
		};

		void destroyUnion();
		void copyUnion(Webss&& o);
		void copyUnion(const Webss& o);
	};
}