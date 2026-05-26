// This header defines the symbol table for the 
// semantic analysis phase of a interpreter. It includes definitions 
// for symbol information, function information, and the 
// symbol table class itself. The symbol table supports 
// scoping, allowing for nested scopes and proper symbol 
// resolution. It also provides functionality to define 
// variables and functions, look up symbols, check for local 
// definitions, and mark symbols as initialized.


#pragma once

#include "core/type.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <optional>
#include <cstdint>


namespace semantics {

enum class symbol_kind : uint8_t {
	VARIABLE,
	FUNCTION
};

struct func_info {
	core::value_type return_type_;
	std::vector<core::value_type> param_types_;
};

struct symbol_info {
	core::value_type type_;
	symbol_kind kind_;
	bool initialized_;
	std::vector<core::value_type> param_types_;
};

class symbol_table {
public:

	symbol_table();

	void push_scope();
	void pop_scope() noexcept;

	void define(const std::string& name, core::value_type type);
	void define_function(const std::string& name, core::value_type return_type,
		const std::vector<core::value_type>& param_types);

	std::optional<symbol_info> lookup(const std::string& name) const;
	
	bool defined_locally(const std::string& name) const;

	void mark_initialized(const std::string& name);

private:

	struct scope {
		std::unordered_map<std::string, symbol_info> symbols_;
	};

	std::vector<std::unique_ptr<scope>> scopes_;

};

} // namespace semantics