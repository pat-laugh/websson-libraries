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
		BasicEntity(std::string&& name, T&& content) : ptr(new EntityBody{ std::move(name), std::move(content) }) {}
		BasicEntity(const std::string& name, const T& content) : ptr(new EntityBody{ name, content }) {}

		const std::string& getName() const { return ptr->name; }
		const T& getContent() const { return ptr->content; }

		bool hasNamespace() const { return ptr->nspace.get() != nullptr; }
		const BasicNamespace<T>& getNamespace() const { return *ptr->nspace; }
		void setNamespace(const std::shared_ptr<BasicNamespace<T>>& nspace) { ptr->nspace = nspace; }
	private:
		class EntityBody
		{
		public:
			std::string name;
			T content;
			std::shared_ptr<BasicNamespace<T>> nspace;
		};

		std::shared_ptr<EntityBody> ptr;
	};
}