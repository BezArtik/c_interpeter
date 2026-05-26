// symbol_table.cpp

// This file implements the symbol table for the semantic analysis phase 
// of the compiler. 


#include "semantics/symbol_table.hpp"
#include <algorithm>

namespace semantics {

symbol_table::symbol_table() {
    scopes_.push_back(std::make_unique<scope>());
}

void symbol_table::push_scope() {
    scopes_.push_back(std::make_unique<scope>());
}

void symbol_table::pop_scope() noexcept {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

void symbol_table::define(const std::string& name, core::value_type type) {
    auto& current = *scopes_.back();
	symbol_info info = { type, symbol_kind::VARIABLE, false, {} };
    current.symbols_[name] = info;
}

void symbol_table::define_function(const std::string& name, core::value_type return_type,
    const std::vector<core::value_type>& param_types) {
    auto& current = *scopes_.back();
	symbol_info info = { return_type, symbol_kind::FUNCTION, true, param_types };
    current.symbols_[name] = info;
}

std::optional<symbol_info> symbol_table::lookup(const std::string& name) const {
	auto it = std::find_if(scopes_.rbegin(), scopes_.rend(),
		[&name](const auto& scope) { return scope->symbols_.find(name) != scope->symbols_.end(); });
	return it != scopes_.rend() ? std::make_optional((*it)->symbols_.at(name)) : std::nullopt;
}

bool symbol_table::defined_locally(const std::string& name) const {
    const auto& symbols = scopes_.back()->symbols_;
    return symbols.find(name) != symbols.end();
}

void symbol_table::mark_initialized(const std::string& name) {
	auto it = std::find_if(scopes_.rbegin(), scopes_.rend(),
		[&name](const auto& scope) { return scope->symbols_.find(name) != scope->symbols_.end(); });
    if (it != scopes_.rend()) {
        (*it)->symbols_[name].initialized_ = true;
        return;
    }
}

} // namespace semantics