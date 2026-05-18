#include "runtime/environment.hpp"
#include <stdexcept>

namespace runtime {

environment::environment() {
    scopes_.push_back(std::make_unique<scope>());
}

void environment::push_scope() {
    scopes_.push_back(std::make_unique<scope>());
}

void environment::pop_scope() {
    if (scopes_.size() > 1) {
        scopes_.pop_back();
    }
}

void environment::define(const std::string& name_, value val) {
    scopes_.back()->values_[name_] = std::move(val);
}

void environment::assign(const std::string& name_, value val) {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        auto& values = (*it)->values_;
        auto found = values.find(name_);
        if (found != values.end()) {
            found->second = std::move(val);
            return;
        }
    }
    throw std::runtime_error("Assignment to undefined variable '" + name_ + "'");
}

std::optional<value> environment::get(const std::string& name_) const {
    for (auto it = scopes_.rbegin(); it != scopes_.rend(); ++it) {
        const auto& values = (*it)->values_;
        auto found = values.find(name_);
        if (found != values.end()) {
            return found->second;
        }
    }
    return std::nullopt;
}

bool environment::contains(const std::string& name_) const {
    return get(name_).has_value();
}

void environment::define_builtin(const std::string& name, builtin_fn fn) {
    builtins_[name] = std::move(fn);
}

std::optional<environment::builtin_fn> environment::get_builtin(const std::string& name) const {
    auto found = builtins_.find(name);
    if (found != builtins_.end()) {
        return found->second;
    }
    return std::nullopt;
}

}