// runtime/environment.cpp

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
#include <optional>
#include <stdexcept>

namespace runtime {

void environment::push_scope() {
    values_.push();
}

void environment::pop_scope() {
    values_.pop();
}

void environment::define(const std::string& name, value val) {
    values_.define(name, val);
}

void environment::assign(const std::string& name, value val) {
    values_.assign(name, val);
}

std::optional<value> environment::get(const std::string& name) const {
    return values_.get(name);
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