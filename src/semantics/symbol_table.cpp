#include "semantics/symbol_table.hpp"

namespace semantics {

symbol_table::symbol_table() {
    scopes_.push_back(std::make_unique<scope>());
}

void symbol_table::push_scope() {
    scopes_.push_back(std::make_unique<scope>());
}

void symbol_table::pop_scope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

void symbol_table::define(const std::string& name, core::value_type type) {
    auto& current = *scopes_.back();
    current.symbols_[name] = symbol_info{ type, false };
}

std::optional<symbol_info> symbol_table::lookup(const std::string& name) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        const auto& symbols = (*it)->symbols_;
        auto found = symbols.find(name);
        if (found != symbols.end()) {
            return found->second; 
        }
    }
    return std::nullopt;
}

bool symbol_table::defined_locally(const std::string& name) const {
    const auto& symbols = scopes_.back()->symbols_;
    return symbols.find(name) != symbols.end();
}

void symbol_table::mark_initialized(const std::string& name) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto& symbols = (*it)->symbols_;
        auto found = symbols.find(name);
        if (found != symbols.end()) {
            found->second.initialized = true;
            return;
        }
    }
}

} 