// semantics/symbol_table.hpp

// This header defines the symbol table for the 
// semantic analysis phase of a interpreter. It includes definitions 
// for symbol information, function information, and the 
// symbol table class itself. The symbol table supports 
// scoping, allowing for nested scopes and proper symbol 
// resolution. It also provides functionality to define 
// variables and functions, look up symbols, check for local 
// definitions, and mark symbols as initialized.


#pragma once
#include "core/token/token.hpp"
#include "core/utils/scoped_map.hpp"
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
	core::value_type return_type_{};
	std::vector<core::value_type> param_types_{};
};

struct symbol_info {
	core::value_type type_{};
	symbol_kind kind_{};
	bool initialized_{};
	std::vector<core::value_type> param_types_{};
};

class symbol_table : public core::scoped_map<symbol_info> {
public:
    using core::scoped_map<symbol_info>::define;

    void define(const std::string& name, core::value_type type);
    void define_function(const std::string& name, core::value_type return_type,
        const std::vector<core::value_type>& param_types);
    void mark_initialized(const std::string& name);
};

} // namespace semantics