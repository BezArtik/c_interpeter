// include/ast/expression.hpp

#pragma once

#include "core/token/token.hpp"
#include <variant>
#include <memory>
#include <vector>

namespace ast {

struct literal_expr;
struct variable_expr;
struct binary_expr;
struct unary_expr;
struct postfix_expr;
struct call_expr;

using expression = std::variant<
    literal_expr,
    variable_expr,
    std::unique_ptr<binary_expr>,
    std::unique_ptr<unary_expr>,
    std::unique_ptr<postfix_expr>,
    std::unique_ptr<call_expr>
>;

struct literal_expr {
    core::token value_{};
    core::line_number line_{};
    core::column_number column_{};

    literal_expr() = default;
    literal_expr(const core::token& value, core::line_number line, core::column_number column)
        : value_(value), line_(line), column_(column) {
    }
};

struct variable_expr {
    core::token name_{};
    core::line_number line_{};
    core::column_number column_{};

    variable_expr() = default;
    variable_expr(const core::token& name, core::line_number line, core::column_number column)
        : name_(name), line_(line), column_(column) {
    }
};

struct binary_expr {
    expression left_; 
    core::token op_{};
    expression right_;
    core::line_number line_{};
    core::column_number column_{};

    binary_expr() = default;
    binary_expr(expression left, const core::token& op, expression right, core::line_number line, core::column_number column)
        : left_(std::move(left)), op_(op), right_(std::move(right)), line_(line), column_(column) {
    }
};

struct unary_expr {
    core::token op_{};
    expression operand_;
    core::line_number line_{};
    core::column_number column_{};

    unary_expr() = default;
    unary_expr(const core::token& op, expression operand, core::line_number line, core::column_number column)
        : op_(op), operand_(std::move(operand)), line_(line), column_(column) {
    }
};

struct postfix_expr {
    expression operand_;
    core::token op_{};
    core::line_number line_{};
    core::column_number column_{};

    postfix_expr() = default;
    postfix_expr(expression operand, core::token op, core::line_number line, core::column_number column)
        : operand_(std::move(operand)), op_(op), line_(line), column_(column) {
    }
};

struct call_expr {
    core::token callee_{};
    std::vector<expression> args_{};
    core::line_number line_{};
    core::column_number column_{};

    call_expr() = default;
    call_expr(const core::token& callee, std::vector<expression> args,
        core::line_number line, core::column_number column)
        : callee_(callee), args_(std::move(args)), line_(line), column_(column) {
    }
};



using expr_ptr = std::unique_ptr<expression>;

} // namespace ast