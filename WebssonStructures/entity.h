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

	template <class T>
	class BasicEntity
	{
	public:
		BasicEntity() {}
		BasicEntity(std::string&& name, T&& content) : ptr(new EntityBody{ std::move(name), std::move(content) }) {}
		BasicEntity(const std::string& name, const T& content) : ptr(new EntityBody{ name, content }) {}

		const std::string& getName() const
		{
			assert(hasBody());
			return ptr->name;
		}
		const T& getContent() const
		{
			assert(hasBody());
			return ptr->content;
		}

		bool hasNamespace() const
		{
			assert(hasBody());
			return ptr->nspace.get() != nullptr;
		}
		const BasicNamespace<T>& getNamespace() const
		{
			assert(hasNamespace());
			return *ptr->nspace;
		}
		void setNamespace(const std::shared_ptr<BasicNamespace<T>>& nspace)
		{
			assert(hasBody() && !hasNamespace());
			ptr->nspace = nspace;
		}
	private:
		class EntityBody
		{
		public:
			std::string name;
			T content;
			std::shared_ptr<BasicNamespace<T>> nspace;
		};

		bool hasBody() const { return ptr.get() != nullptr; }

		std::shared_ptr<EntityBody> ptr;
	};
}