#include "semantics/type_check.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "core/overloaded.hpp"
#include <string>

namespace semantics {

type_checker::type_checker(core::error_reporter& reporter)
    : reporter_(reporter) {
}

bool type_checker::check(const std::vector<std::unique_ptr<ast::statement>>& statements_) {
    for (const auto& stmt : statements_) {
        check_statement(*stmt);
    }
    return !reporter_.has_error();
}

void type_checker::check_statement(const ast::statement& stmt) {
    std::visit(core::overloaded{
        [this](const ast::expression_stmt& s) { check_expression_stmt(s); },
        [this](const ast::var_declaration& s) { check_var_declaration(s); },
        [this](const ast::block_stmt& s) { check_block(s); },
        [this](const ast::while_stmt& s) { check_while(s); },
        [this](const ast::if_stmt& s) { check_if(s); },
        }, stmt.data_);
}

void type_checker::check_expression_stmt(const ast::expression_stmt& stmt) {
    type_of(*stmt.expr_);
}

void type_checker::check_var_declaration(const ast::var_declaration& stmt) {
    std::string name_{ stmt.name_.lexeme_ };

    if (symbols_.defined_locally(name_)) {
        reporter_.error(stmt.name_.line_, stmt.name_.column_,
            "redeclaration of variable '" + name_ + "'");
        return;
    }

    if (stmt.initializer_ != nullptr) {
        core::value_type init_type = type_of(*stmt.initializer_);
        if (init_type == core::value_type::UNKNOWN) {
            return;
        }
        if (init_type != stmt.type_) {
            reporter_.error(stmt.name_.line_, stmt.name_.column_,
                "type mismatch in initialisation of '" + name_ + "'");
            return;
        }
    }

    symbols_.define(name_, stmt.type_);
    if (stmt.initializer_ != nullptr) {
        symbols_.mark_initialized(name_);
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

void type_checker::check_if(const ast::if_stmt& stmt) {
    core::value_type cond_type = type_of(*stmt.condition_);
    if (cond_type != core::value_type::BOOL && cond_type != core::value_type::UNKNOWN) {
        reporter_.error(0, 0, "if condition must be a boolean expression");
    }
    check_statement(*stmt.then_branch_);
    if (stmt.else_branch_ != nullptr) {
        check_statement(*stmt.else_branch_);
    }
}

core::value_type type_checker::type_of(const ast::expression& expr_) {
    return std::visit(core::overloaded{
        [this](const ast::literal_expr& e) { return type_of_literal(e); },
        [this](const ast::variable_expr& e) { return type_of_variable(e); },
        [this](const ast::binary_expr& e) { return type_of_binary(e); },
        [this](const ast::unary_expr& e) { return type_of_unary(e); },
        }, expr_.data_);
}

core::value_type type_checker::type_of_literal(const ast::literal_expr& expr_) {
    const auto& token = expr_.value_;
    switch (token.type_) {
    case core::token_type::NUMBER: {
        std::string_view lex = token.lexeme_;
        bool is_double = (lex.find('.') != std::string_view::npos ||
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

core::value_type type_checker::type_of_variable(const ast::variable_expr& expr_) {
    std::string name_{ expr_.name_.lexeme_ };
    auto info = symbols_.lookup(name_);
    if (!info) {
        reporter_.error(expr_.name_.line_, expr_.name_.column_,
            "undefined variable '" + name_ + "'");
        return core::value_type::UNKNOWN;
    }
    return info->type_;
}

core::value_type type_checker::type_of_binary(const ast::binary_expr& expr_) {
    core::value_type left = type_of(*expr_.left_);
    core::value_type right = type_of(*expr_.right_);
    if (left == core::value_type::UNKNOWN || right == core::value_type::UNKNOWN) {
        return core::value_type::UNKNOWN;
    }

    auto is_numeric = [](core::value_type t) {
        return t == core::value_type::INT || t == core::value_type::DOUBLE;
        };

    core::token_type op = expr_.op_.type_;

    if (op == core::token_type::PLUS || op == core::token_type::MINUS ||
        op == core::token_type::STAR || op == core::token_type::SLASH ||
        op == core::token_type::PERCENT) {
        if (!is_numeric(left) || !is_numeric(right)) {
            reporter_.error(expr_.op_.line_, expr_.op_.column_,
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
            reporter_.error(expr_.op_.line_, expr_.op_.column_,
                "comparison requires numeric operands");
            return core::value_type::UNKNOWN;
        }
        return core::value_type::BOOL;
    }

    if (op == core::token_type::AND || op == core::token_type::OR) {
        if (left != core::value_type::BOOL || right != core::value_type::BOOL) {
            reporter_.error(expr_.op_.line_, expr_.op_.column_,
                "logical operators require boolean operands");
            return core::value_type::UNKNOWN;
        }
        return core::value_type::BOOL;
    }

    reporter_.error(expr_.op_.line_, expr_.op_.column_, "unsupported binary operator");
    return core::value_type::UNKNOWN;
}

core::value_type type_checker::type_of_unary(const ast::unary_expr& expr_) {
    core::value_type operand_type = type_of(*expr_.operand_);
    if (operand_type == core::value_type::UNKNOWN) {
        return core::value_type::UNKNOWN;
    }

    core::token_type op = expr_.op_.type_;

    if (op == core::token_type::MINUS) {
        if (operand_type != core::value_type::INT && operand_type != core::value_type::DOUBLE) {
            reporter_.error(expr_.op_.line_, expr_.op_.column_,
                "unary minus requires numeric operand");
            return core::value_type::UNKNOWN;
        }
        return operand_type;
    }

    if (op == core::token_type::BANG) {
        if (operand_type != core::value_type::BOOL) {
            reporter_.error(expr_.op_.line_, expr_.op_.column_,
                "logical not requires boolean operand");
            return core::value_type::UNKNOWN;
        }
        return core::value_type::BOOL;
    }

    reporter_.error(expr_.op_.line_, expr_.op_.column_, "unsupported unary operator");
    return core::value_type::UNKNOWN;
}

}