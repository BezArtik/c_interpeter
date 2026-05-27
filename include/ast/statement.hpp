// ast/statement.hpp

// This file defines the various statement types in the AST, 
// such as variable declarations, 
// control flow statements, and function declarations.


#pragma once

#include "ast/expression.hpp"
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

struct for_stmt {
	std::unique_ptr<struct statement> initializer_;
	std::unique_ptr<expression> condition_;
	std::unique_ptr<expression> increment_;
	std::unique_ptr<struct statement> body_;
};

struct if_stmt {
    std::unique_ptr<expression> condition_;
    std::unique_ptr<struct statement> then_branch_;
    std::unique_ptr<struct statement> else_branch_;
};

struct return_stmt {
    core::token keyword_;
    std::unique_ptr<expression> value_;
};

struct func_param {
    core::value_type type_;
    core::token name_;
};

struct func_declaration {
    core::value_type return_type_;
    core::token name_;
    std::vector<func_param> params_;
    std::unique_ptr<block_stmt> body_;
};

struct statement {
    std::variant<
        expression_stmt,
        var_declaration,
        block_stmt,
        for_stmt,
        while_stmt,
        if_stmt,
        return_stmt,
        func_declaration
    > data_;

    statement() : data_(expression_stmt{}) {}

	template <typename T>
	statement(T s) : data_(std::move(s)) {}
};

} // namespace ast