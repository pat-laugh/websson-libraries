//MIT License
//Copyright(c) 2016 Patrick Laughrea
#pragma once

#include <string>
#include <memory>

namespace webss
{
	template <typename T>
	class BasicVariable
	{
	public:
		BasicVariable() {}
		BasicVariable(std::string&& name, T&& content) : ptr(new VariableBody<T>(std::move(name), std::move(content))) {}
		BasicVariable(const std::string& name, const T& content) : ptr(new VariableBody<T>(name, content)) {}

		~BasicVariable() {}

		BasicVariable(BasicVariable&& var) : ptr(std::move(var.ptr)) {}
		BasicVariable(const BasicVariable& var) : ptr(var.ptr) {}

		BasicVariable& operator=(BasicVariable&& o)
		{
			if (this != &o)
				ptr = std::move(o.ptr);
			return *this;
		}
		BasicVariable& operator=(const BasicVariable& o)
		{
			if (this != &o)
				ptr = o.ptr;
			return *this;
		}

		const std::string& getName() const { return ptr->name; }
		const T& getContent() const { return ptr->content; }

//		VariableBody<T>* operator->() { return ptr.get(); }
//		const VariableBody<T>* operator->() const { return ptr.get(); }
//		VariableBody<T>& operator*() { return *ptr; }
//		const VariableBody<T>& operator*() const { return *ptr; }
	private:
		template <typename T>
		struct VariableBody
		{
			std::string name;
			T content;

			VariableBody(std::string&& name, T&& content) : name(std::move(name)), content(std::move(content)) {}
			VariableBody(const std::string& name, const T& content) : name(name), content(content) {}
		};

		std::shared_ptr<VariableBody<T>> ptr;
	};
}