#pragma once

#include "WebssonStructures/webss.h"
#include "entityManager.h"

namespace webss
{
	class MultiEntityManager
	{
	private:
		static constexpr char* ERROR_ENTITY_EXISTS = "entity already exists: ";
		using Entity = BasicEntity<Webss>;
		using EntityWebss = BasicEntity<Webss>;
		using EntityBlockHead = BasicEntity<BlockHead>;
		using EntityFheadBinary = BasicEntity<FunctionHeadBinary>;
		using EntityFheadScoped = BasicEntity<FunctionHeadScoped>;
		using EntityFheadStandard = BasicEntity<FunctionHeadStandard>;

		BasicEntityManager<Webss> entsWebss;
		BasicEntityManager<BlockHead> entsBlockHead;
		BasicEntityManager<FunctionHeadBinary> entsFheadBinary;
		BasicEntityManager<FunctionHeadScoped> entsFheadScoped;
		BasicEntityManager<FunctionHeadStandard> entsFheadStandard;
	public:
		MultiEntityManager() {}
		~MultiEntityManager() { clearAll(); }

		void clearAll()
		{
			entsWebss.clearAll();
			entsBlockHead.clearAll();
			entsFheadBinary.clearAll();
			entsFheadScoped.clearAll();
			entsFheadStandard.clearAll();
		}

		void clearLocals()
		{
			entsWebss.clearLocals();
			entsBlockHead.clearLocals();
			entsFheadBinary.clearLocals();
			entsFheadScoped.clearLocals();
			entsFheadStandard.clearLocals();
		}

		void addGlobal(std::string name, Webss content)
		{
			addGlobal(Entity(std::move(name), std::move(content)));
		}
		void addGlobalSafe(std::string name, Webss content)
		{
			addGlobalSafe(Entity(std::move(name), std::move(content)));
		}
		void addGlobal(Entity ent)
		{
			const auto& name = ent.getName();
			const auto& content = ent.getContent();
			switch (content.getType())
			{
			case WebssType::BLOCK_HEAD:
				entsBlockHead.addGlobal(name, content.getBlockHead());
				break;
			case WebssType::FUNCTION_HEAD_BINARY:
				entsFheadBinary.addGlobal(name, content.getFunctionHeadBinary());
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				entsFheadScoped.addGlobal(name, content.getFunctionHeadScoped());
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				entsFheadStandard.addGlobal(name, content.getFunctionHeadStandard());
				break;
			default:
				break;
			}

			entsWebss.addGlobal(std::move(ent));
		}
		void addGlobalSafe(Entity ent)
		{
			if (hasEntity(ent.getName()))
				throw std::runtime_error(ERROR_ENTITY_EXISTS + ent.getName());

			addGlobal(std::move(ent));
		}

		void addLocal(std::string name, Webss content)
		{
			addLocal(Entity(std::move(name), std::move(content)));
		}
		void addLocalSafe(std::string name, Webss content)
		{
			addLocalSafe(Entity(std::move(name), std::move(content)));
		}
		void addLocal(Entity ent)
		{
			const auto& name = ent.getName();
			const auto& content = ent.getContent();
			switch (content.getType())
			{
			case WebssType::BLOCK_HEAD:
				entsBlockHead.addLocal(name, content.getBlockHead());
				break;
			case WebssType::FUNCTION_HEAD_BINARY:
				entsFheadBinary.addLocal(name, content.getFunctionHeadBinary());
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				entsFheadScoped.addLocal(name, content.getFunctionHeadScoped());
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				entsFheadStandard.addLocal(name, content.getFunctionHeadStandard());
				break;
			default:
				break;
			}

			entsWebss.addLocal(std::move(ent));
		}
		void addLocalSafe(Entity ent)
		{
			if (hasEntity(ent.getName()))
				throw std::runtime_error(ERROR_ENTITY_EXISTS + ent.getName());

			addLocal(std::move(ent));
		}

		//returns true if s is a entity, else false
		bool hasEntity(const std::string& s) const { return entsWebss.hasEntity(s); }

		const EntityWebss& getWebss(const std::string& name) { return entsWebss[name]; }
		const EntityBlockHead& getBlockHead(const std::string& name) { return entsBlockHead[name]; }
		const EntityFheadBinary& getFheadBinary(const std::string& name) { return entsFheadBinary[name]; }
		const EntityFheadScoped& getFheadScoped(const std::string& name) { return entsFheadScoped[name]; }
		const EntityFheadStandard& getFheadStandard(const std::string& name) { return entsFheadStandard[name]; }

		const EntityWebss& getWebssSafe(const std::string& name) { return entsWebss.at(name); }
		const EntityBlockHead& getBlockHeadSafe(const std::string& name) { return entsBlockHead.at(name); }
		const EntityFheadBinary& getFheadBinarySafe(const std::string& name) { return entsFheadBinary.at(name); }
		const EntityFheadScoped& getFheadScopedSafe(const std::string& name) { return entsFheadScoped.at(name); }
		const EntityFheadStandard& getFheadStandardSafe(const std::string& name) { return entsFheadStandard.at(name); }

		void removeGlobal(const std::string& name)
		{
			switch (getWebss(name).getContent().getType())
			{
			case WebssType::BLOCK_HEAD:
				entsBlockHead.removeGlobal(name);
				break;
			case WebssType::FUNCTION_HEAD_BINARY:
				entsFheadBinary.removeGlobal(name);
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				entsFheadScoped.removeGlobal(name);
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				entsFheadStandard.removeGlobal(name);
				break;
			default:
				break;
			}

			entsWebss.removeGlobal(name);
		}
		void removeLocal(const std::string& name)
		{
			switch (getWebss(name).getContent().getType())
			{
			case WebssType::BLOCK_HEAD:
				entsBlockHead.removeLocal(name);
				break;
			case WebssType::FUNCTION_HEAD_BINARY:
				entsFheadBinary.removeLocal(name);
				break;
			case WebssType::FUNCTION_HEAD_SCOPED:
				entsFheadScoped.removeLocal(name);
				break;
			case WebssType::FUNCTION_HEAD_STANDARD:
				entsFheadStandard.removeLocal(name);
				break;
			default:
				break;
			}

			entsWebss.removeLocal(name);
		}

		void removeGlobal(const Entity& ent)
		{
			removeGlobal(ent.getName());
		}
		void removeLocal(const Entity& ent)
		{
			removeLocal(ent.getName());
		}
	};
}