// semantics/symbol_table.cpp

// This file implements the symbol table for the semantic analysis phase 
// of the compiler. 


#include "semantics/symbol_table.hpp"
#include <algorithm>

namespace semantics {

void symbol_table::define(const std::string& name, core::value_type type) {
    symbol_info info{ type, symbol_kind::VARIABLE, false, {} };
    define(name, std::move(info));
}

void symbol_table::define_function(const std::string& name, core::value_type return_type,
    const std::vector<core::value_type>& param_types) {
    symbol_info info{ return_type, symbol_kind::FUNCTION, true, param_types };
    define(name, std::move(info));
}

void symbol_table::mark_initialized(const std::string& name) {
    update_if_exists(name, [](auto& info) {
        info.initialized_ = true;
        });
}

} // namespace semantics