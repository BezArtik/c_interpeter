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
    symbol_info info;
    info.kind_ = symbol_kind::VARIABLE;
    info.type_ = type_;
    info.initialized_ = false;
    current.symbols_[name_] = info;
}

void symbol_table::define_function(const std::string& name, core::value_type return_type,
    const std::vector<core::value_type>& param_types) {
    auto& current = *scopes_.back();
    symbol_info info;
    info.kind_ = symbol_kind::FUNCTION;
    info.type_ = return_type;
    info.param_types_ = param_types;
    info.initialized_ = true;
    current.symbols_[name] = info;
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
            found->second.initialized_ = true;
            return;
        }
    }
}

} 