//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <string>
#include <memory>

namespace webss
{
	template <typename T>
	class BasicEntity
	{
	public:
		BasicEntity() {}
		BasicEntity(std::string&& name, T&& content) : ptr(new EntityBody<T>(std::move(name), std::move(content))) {}
		BasicEntity(const std::string& name, const T& content) : ptr(new EntityBody<T>(name, content)) {}

		~BasicEntity() {}

		BasicEntity(BasicEntity&& ent) : ptr(std::move(ent.ptr)) {}
		BasicEntity(const BasicEntity& ent) : ptr(ent.ptr) {}

		BasicEntity& operator=(BasicEntity&& o)
		{
			if (this != &o)
				ptr = std::move(o.ptr);
			return *this;
		}
		BasicEntity& operator=(const BasicEntity& o)
		{
			if (this != &o)
				ptr = o.ptr;
			return *this;
		}

		const std::string& getName() const { return ptr->name; }
		const T& getContent() const { return ptr->content; }

//		EntityBody<T>* operator->() { return ptr.get(); }
//		const EntityBody<T>* operator->() const { return ptr.get(); }
//		EntityBody<T>& operator*() { return *ptr; }
//		const EntityBody<T>& operator*() const { return *ptr; }
	private:
		template <typename T>
		struct EntityBody
		{
			std::string name;
			T content;

			EntityBody(std::string&& name, T&& content) : name(std::move(name)), content(std::move(content)) {}
			EntityBody(const std::string& name, const T& content) : name(name), content(content) {}
		};

		std::shared_ptr<EntityBody<T>> ptr;
	};
}