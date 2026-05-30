// core/builtins.hpp

// This file defines the built-in functions 
// available in the language. Each built-in 
// function is represented by a builtin_def struct, 
// which includes the function's name, its overloads 
// (parameter types), return type, and implementation. 
// The builtins() function returns a list of all built-in 
// unctions that can be registered in the environment.


#pragma once
#include "core/token/token.hpp"
#include "core/token/token_types.hpp"
#include <string>
#include <vector>
#include <functional>


namespace runtime { class value; class environment; }

namespace core {

struct builtin_def {
    std::string name_;
    std::vector<std::vector<type>> overloads_; 
    type return_type_;
    std::function<runtime::value(const std::vector<runtime::value>&)> impl_;
};

std::vector<builtin_def> builtins();

} // namespace core
