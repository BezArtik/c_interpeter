#include "semantics/type_check.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include <string>

namespace semantics {

type_checker::type_checker(core::error_reporter& reporter)
    : reporter_(reporter) {
}

bool type_checker::check(const std::vector<std::unique_ptr<ast::statement>>& statements) {
    for (const auto& stmt : statements) {
        check_statement(*stmt);
    }
    return !reporter_.has_error();
}

void type_checker::check_statement(const ast::statement& stmt) {
    if (auto* p = dynamic_cast<const ast::expression_stmt*>(&stmt)) {
        check_expression_stmt(*p);
    } else if (auto* p = dynamic_cast<const ast::var_declaration*>(&stmt)) {
        check_var_declaration(*p);
    } else if (auto* p = dynamic_cast<const ast::block_stmt*>(&stmt)) {
        check_block(*p);
    } else if (auto* p = dynamic_cast<const ast::while_stmt*>(&stmt)) {
        check_while(*p);
    }
}

void type_checker::check_expression_stmt(const ast::expression_stmt& stmt) {
    type_of(*stmt.expr_); 
}

void type_checker::check_var_declaration(const ast::var_declaration& stmt) {
    std::string name{ stmt.name_.lexeme_ }; 

    if (symbols_.defined_locally(name)) {
        reporter_.error(stmt.name_.line_, stmt.name_.column_,
            "redeclaration of variable '" + name + "'");
        return;
    }

    if (stmt.initializer_) {
        core::value_type init_type = type_of(*stmt.initializer_);
        if (init_type == core::value_type::UNKNOWN) {
            return;
        }

        if (init_type != stmt.type_) {
            reporter_.error(stmt.name_.line_, stmt.name_.column_,
                "type mismatch in initialisation of '" + name + "'");
            return;
        }
    }

    symbols_.define(name, stmt.type_);
    if (stmt.initializer_) {
        symbols_.mark_initialized(name);
    }
}

void type_checker::check_block(const ast::block_stmt& stmt) {
    symbols_.push_scope();
    for (const auto& s : stmt.statements_) {
        check_statement(*s);
    }
    symbols_.pop_scope();
}

void type_checker::check_while(const ast::while_stmt& stmt) {
    core::value_type cond_type = type_of(*stmt.condition_);
    if (cond_type != core::value_type::BOOL && cond_type != core::value_type::UNKNOWN) {
        reporter_.error(0, 0, "while condition must be a boolean expression");
    }
    check_statement(*stmt.body_);
}

core::value_type type_checker::type_of(const ast::expression& expr) {
    if (auto* p = dynamic_cast<const ast::literal_expr*>(&expr)) {
        return type_of_literal(*p);
    } else if (auto* p = dynamic_cast<const ast::variable_expr*>(&expr)) {
        return type_of_variable(*p);
    } else if (auto* p = dynamic_cast<const ast::binary_expr*>(&expr)) {
        return type_of_binary(*p);
    }
    reporter_.error(0, 0, "unknown expression type");
    return core::value_type::UNKNOWN;
}

core::value_type type_checker::type_of_literal(const ast::literal_expr& expr) {
    const auto& token = expr.value_;
    switch (token.type_) {
    case core::token_type::NUMBER: {
        std::string_view lex = token.lexeme_;
        bool is_double = 
            (lex.find('.') != std::string_view::npos ||
            lex.find('e') != std::string_view::npos ||
            lex.find('E') != std::string_view::npos);
        return is_double ? core::value_type::DOUBLE : core::value_type::INT;
    }
    case core::token_type::TRUE:
    case core::token_type::FALSE:
        return core::value_type::BOOL;
    case core::token_type::STRING:
        return core::value_type::STRING;
    default:
        reporter_.error(token.line_, token.column_, "unexpected literal");
        return core::value_type::UNKNOWN;
    }
}

core::value_type type_checker::type_of_variable(const ast::variable_expr& expr) {
    std::string name{ expr.name_.lexeme_ };
    auto info = symbols_.lookup(name);
    if (!info) {
        reporter_.error(expr.name_.line_, expr.name_.column_,
            "undefined variable '" + name + "'");
        return core::value_type::UNKNOWN;
    }
    return info->type_;
}

core::value_type type_checker::type_of_binary(const ast::binary_expr& expr) {
    core::value_type left = type_of(*expr.left_);
    core::value_type right = type_of(*expr.right_);
    if (left == core::value_type::UNKNOWN || right == core::value_type::UNKNOWN) {
        return core::value_type::UNKNOWN;
    }

    auto is_numeric = [](core::value_type t) {
        return t == core::value_type::INT || t == core::value_type::DOUBLE;
        };

    core::token_type op = expr.operator_.type_;

    if (op == core::token_type::PLUS || op == core::token_type::MINUS ||
        op == core::token_type::STAR || op == core::token_type::SLASH) {
        if (!is_numeric(left) || !is_numeric(right)) {
            reporter_.error(expr.operator_.line_, expr.operator_.column_,
                "arithmetic requires numeric operands");
            return core::value_type::UNKNOWN;
        }
        return (left == core::value_type::INT && right == core::value_type::INT)
            ? core::value_type::INT : core::value_type::DOUBLE;
    }

    if (op == core::token_type::EQUAL_EQUAL || op == core::token_type::BANG_EQUAL ||
        op == core::token_type::LESS || op == core::token_type::LESS_EQUAL ||
        op == core::token_type::GREATER || op == core::token_type::GREATER_EQUAL) {
        if (!is_numeric(left) || !is_numeric(right)) {
            reporter_.error(expr.operator_.line_, expr.operator_.column_,
                "comparison requires numeric operands");
            return core::value_type::UNKNOWN;
        }
        return core::value_type::BOOL;
    }

    reporter_.error(expr.operator_.line_, expr.operator_.column_,
        "unsupported binary operator");
    return core::value_type::UNKNOWN;
}

}