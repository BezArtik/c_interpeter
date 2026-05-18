#pragma once

#include "ast/expression.hpp"
#include "core/type.hpp"
#include <vector>
#include <variant>
#include <memory>

namespace ast {

struct expression_stmt {
    std::unique_ptr<expression> expr_;
};

struct var_declaration {
    core::value_type type_;
    core::token name_;
    std::unique_ptr<expression> initializer_;
};

struct block_stmt {
    std::vector<std::unique_ptr<struct statement>> statements_;
};

struct while_stmt {
    std::unique_ptr<expression> condition_;
    std::unique_ptr<struct statement> body_;
};

struct if_stmt {
    std::unique_ptr<expression> condition_;
    std::unique_ptr<struct statement> then_branch_;
    std::unique_ptr<struct statement> else_branch_;
};

struct return_stmt {
    core::token keyword_;
    std::unique_ptr<struct expression> value_;
};

struct func_param {
    core::value_type type_;
    core::token name_;
};

struct func_declaration {
    core::value_type return_type_;
    core::token name_;
    std::vector<func_param> params_;
    std::unique_ptr<statement> body_;
};

struct statement {
    std::variant<
        expression_stmt,
        var_declaration,
        block_stmt,
        while_stmt,
        if_stmt,
        return_stmt,
        func_declaration
    > data_;

    statement();
    statement(expression_stmt s);
    statement(var_declaration s);
    statement(block_stmt s);
    statement(while_stmt s);
    statement(if_stmt s);
    statement(return_stmt s);
    statement(func_declaration s);
};

}