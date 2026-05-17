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

void symbol_table::define(const std::string& name_, core::value_type type_) {
    auto& current = *scopes_.back();
    current.symbols_[name_] = symbol_info{ type_, false };
}

std::optional<symbol_info> symbol_table::lookup(const std::string& name_) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        const auto& symbols = (*it)->symbols_;
        auto found = symbols.find(name_);
        if (found != symbols.end()) {
            return found->second; 
        }
    }
    return std::nullopt;
}

bool symbol_table::defined_locally(const std::string& name_) const {
    const auto& symbols = scopes_.back()->symbols_;
    return symbols.find(name_) != symbols.end();
}

void symbol_table::mark_initialized(const std::string& name_) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto& symbols = (*it)->symbols_;
        auto found = symbols.find(name_);
        if (found != symbols.end()) {
            found->second.initialized = true;
            return;
        }
    }
}

} 