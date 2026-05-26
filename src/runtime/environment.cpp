// environment.cpp

// This code implements the `environment` class, 
// which manages variable scopes and built-in functions 
// for a runtime environment. The class allows for defining and 
// assigning variables, as well as retrieving their values. 
// It also supports defining and retrieving built-in functions. 
// The implementation uses a vector of unique pointers to manage multiple scopes, 
// allowing for nested variable definitions. The `get` method searches through 
// the scopes from the innermost to the outermost to find a variable, while the
// `assign` method updates an existing variable or throws an error if it is not defined.


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

void environment::define(const std::string& name, value val) {
    scopes_.back()->values_[name] = std::move(val);
}

void environment::assign(const std::string& name, value val) {
	auto it = std::find_if(scopes_.rbegin(), scopes_.rend(),
		[&name](const auto& s) {
		return s->values_.find(name) != s->values_.end();
		});
    if (it != scopes_.rend()) {
        (*it)->values_.at(name) = std::move(val);
        return;
    }
    throw std::runtime_error("Assignment to undefined variable '" + name + "'");
}

std::optional<value> environment::get(const std::string& name) const {
	auto it = std::find_if(scopes_.rbegin(), scopes_.rend(), 
        [&name](const auto& s) {
		return s->values_.find(name) != s->values_.end();
		});
    if (it != scopes_.rend()) return (*it)->values_.at(name);
    return std::nullopt;
}

bool environment::contains(const std::string& name) const {
    return get(name).has_value();
}

void environment::define_builtin(const std::string& name, builtin_fn fn) {
    builtins_[name] = std::move(fn);
}

std::optional<environment::builtin_fn> environment::get_builtin(const std::string& name) const {
    auto found = builtins_.find(name);
    if (found != builtins_.end()) return found->second;
    return std::nullopt;
}

} // namespace runtime