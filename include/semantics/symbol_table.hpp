#pragma once

#include "core/type.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>


namespace semantics {

struct symbol_info {
	core::value_type type_ = core::value_type::UNKNOWN;
	bool initialized = false;
};

class symbol_table {
public:

	symbol_table();

	void push_scope();
	void pop_scope();

	void define(const std::string& name, core::value_type type);

	std::optional<symbol_info> lookup(const std::string& name) const;
	
	bool defined_locally(const std::string& name) const;

	void mark_initialized(const std::string& name);

private:

	struct scope {
		std::unordered_map<std::string, symbol_info> symbols_;
	};

	std::vector<std::unique_ptr<scope>> scopes_;

};

}