#pragma once

#include "ast/ast_node.hpp"
#include "core/token.hpp"
#include "core/type.hpp"
#include <memory>

namespace ast {

class literal_expr : public expression {
public:
    literal_expr(core::token value);

    core::token value_; 
    core::value_type type_;
};

class binary_expr : public expression {
public:
    binary_expr(std::unique_ptr<expression> left, core::token op, std::unique_ptr<expression> right);

    std::unique_ptr<expression> left_;
    core::token operator_;
    std::unique_ptr<expression> right_;
};

class variable_expr : public expression {
public:
    variable_expr(core::token name);

    core::token name_;
};

}
