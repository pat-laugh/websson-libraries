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
		BasicEntity(std::string&& name, T&& content) : ptrName(new std::string(std::move(name))), ptrContent(new T(std::move(content))) {}
		BasicEntity(const std::string& name, const T& content) : ptrName(new std::string(name)), ptrContent(new T(content)) {}

		~BasicEntity() {}

		BasicEntity(BasicEntity&& ent) : ptrName(std::move(ent.ptrName)), ptrContent(std::move(ent.ptrContent)) {}
		BasicEntity(const BasicEntity& ent) : ptrName(ent.ptrName), ptrContent(ent.ptrContent) {}

		BasicEntity& operator=(BasicEntity&& o)
		{
			if (this != &o)
			{
				ptrName = std::move(o.ptrName);
				ptrContent = std::move(o.ptrContent);
			}
			return *this;
		}
		BasicEntity& operator=(const BasicEntity& o)
		{
			if (this != &o)
			{
				ptrName = o.ptrName;
				ptrContent = o.ptrContent;
			}
			return *this;
		}

		const std::string& getName() const { return *ptrName; }
		const T& getContent() const { return *ptrContent; }

		BasicEntity copyContent(std::string&& newName) const
		{
			return BasicEntity(*this, std::move(newName));
		}
	private:
		std::shared_ptr<std::string> ptrName;
		std::shared_ptr<T> ptrContent;

		BasicEntity(const BasicEntity& ent, std::string&& newName) : ptrName(new std::string(std::move(newName))), ptrContent(ent.ptrContent) {}
	};
}