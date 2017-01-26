//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <string>
#include <memory>

#include "base.h"

namespace webss
{
	template <class Webss>
	class BasicNamespace;

	template <typename T>
	class BasicEntity
	{
	public:
		BasicEntity() {}
		BasicEntity(std::string&& name, T&& content) : ptrName(new std::string(std::move(name))), ptrContent(new T(std::move(content))) {}
		BasicEntity(const std::string& name, const T& content) : ptrName(new std::string(name)), ptrContent(new T(content)) {}

		const std::string& getName() const { return *ptrName; }
		const T& getContent() const { return *ptrContent; }

		BasicEntity copyContent(std::string&& newName) const { return BasicEntity(*this, std::move(newName)); }

		bool hasNamespace() const { return ptrNamespace.get() != nullptr; }
		const BasicNamespace<T>& getNamespace() const { return *ptrNamespace; }
		void setNamespace(const std::shared_ptr<BasicNamespace<T>>& nspace) { ptrNamespace = nspace; }
	private:
		std::shared_ptr<std::string> ptrName;
		std::shared_ptr<T> ptrContent;
		std::shared_ptr<BasicNamespace<T>> ptrNamespace;

		BasicEntity(const BasicEntity& ent, std::string&& newName) : ptrName(new std::string(std::move(newName))), ptrContent(ent.ptrContent) {}
	};
}