// The environment class manages variable scopes 
// and built-in functions for the runtime.


#pragma once

#include "runtime/value.hpp"
#include "core/type.hpp"
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <functional>


namespace runtime {

class environment {
public:
    using builtin_fn = std::function<value(const std::vector<value>&)>;

    environment();

    void push_scope();

    void pop_scope();

    void define(const std::string& name, value val);
    void assign(const std::string& name, value val);
    std::optional<value> get(const std::string& name) const;
    bool contains(const std::string& name) const;

    void define_builtin(const std::string& name, builtin_fn fn);
    std::optional<builtin_fn> get_builtin(const std::string& name) const;

private:

    struct scope {
        std::unordered_map<std::string, value> values_;
    };
    std::vector<std::unique_ptr<scope>> scopes_;
    std::unordered_map<std::string, builtin_fn> builtins_;
};

} // namespace runtime