//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include "document.h"

namespace webss
{
	template <class Webss>
	class BasicFolder
	{
	public:
		using Document = BasicDocument<Webss>;
		using Pair = std::pair<std::string, Document>;
		using Data = std::vector<Pair>;

		BasicFolder() {}
		~BasicFolder() {}

		BasicFolder(BasicFolder&& o) : data(std::move(o.data)), mainDoc(std::move(o.mainDoc)) {}
		BasicFolder(const BasicFolder& o) : data(o.data), mainDoc(o.mainDoc) {}

		BasicFolder& operator=(BasicFolder&& o)
		{
			if (this != &o)
			{
				data = std::move(o.data);
				mainDoc = std::move(o.mainDoc);
			}
			return *this;
		}
		BasicFolder& operator=(const BasicFolder& o)
		{
			if (this != &o)
			{
				data = o.data;
				mainDoc = o.mainDoc;
			}
			return *this;
		}

		bool empty() const { return data.empty(); }
		typename Data::size_type size() const { return data.size(); }

		void addDocument(std::string&& key, Document&& doc) { data.push_back({ std::move(key), std::move(doc) }); }
		void addDocument(const std::string& key, const Document& doc) { data.push_back({ key, doc }); }
		void addDocumentSafe(std::string&& key, Document&& doc)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY + key);

			addDocument(std::move(key), std::move(doc));
		}
		void addDocumentSafe(const std::string& key, const Document& doc)
		{
			if (has(key))
				throw std::runtime_error(ERROR_DUPLICATE_KEY + key);

			addDocument(key, doc);
		}

		Document& getMainDocument() { return mainDoc; }
		const Document& getMainDocument() const { return mainDoc; }

		bool has(const std::string& key) const
		{
			for (const auto& keyValue : data)
				if (keyValue.first == key)
					return true;
			return false;
		}

		Pair& operator[](typename Data::size_type index) { return data[index]; }
		const Pair& operator[](typename Data::size_type index) const { return data[index]; }
		Pair& at(typename Data::size_type index) { return data.at(index); }
		const Pair& at(typename Data::size_type index) const { return data.at(index); }

		typename Data::iterator begin() { return data.begin(); }
		typename Data::iterator end() { return data.end(); }
		typename Data::const_iterator begin() const { return data.begin(); }
		typename Data::const_iterator end() const { return data.end(); }
		typename Data::reverse_iterator rbegin() { return data.rbegin(); }
		typename Data::reverse_iterator rend() { return data.rend(); }
		typename Data::const_reverse_iterator rbegin() const { return data.rbegin(); }
		typename Data::const_reverse_iterator rend() const { return data.rend(); }
	private:
		static constexpr char* ERROR_DUPLICATE_KEY = "document already imported: ";

		Data data;
		Document mainDoc;
	};
}