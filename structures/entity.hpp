//MIT License
//Copyright 2017 Patrick Laughrea
#pragma once

#include <memory>
#include <string>

#include "namespace.hpp"

namespace webss
{
	class Entity
	{
	public:
		Entity();
		Entity(std::string name, Webss content);

		bool hasBody() const;
		const std::string& getName() const;
		const Webss& getContent() const;

		bool operator==(const Entity& o) const;
		bool operator!=(const Entity& o) const;

		bool hasNamespace() const;
		const Namespace& getNamespace() const;
		void setNamespace(const Namespace& nspace);

	private:
		struct EntityBody;

		std::shared_ptr<EntityBody> ptr;
	};
}