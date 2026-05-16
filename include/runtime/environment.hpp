#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include "runtime/value.hpp"

namespace runtime {

class environment {
public:
    environment();

    void push_scope();

    void pop_scope();

    void define(const std::string& name, value val);
    void assign(const std::string& name, value val);
    std::optional<value> get(const std::string& name) const;
    bool contains(const std::string& name) const;

private:
    struct scope {
        std::unordered_map<std::string, value> values_;
    };
    std::vector<std::unique_ptr<scope>> scopes_;
};

}