#include "runtime/interpreter.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "core/overloaded.hpp"
#include <stdexcept>
#include <string>

namespace runtime {

interpreter::interpreter(core::error_reporter& reporter)
    : reporter_(reporter)
    , global_env_(std::make_unique<environment>())
    , current_env_(global_env_.get()) {
}

void interpreter::interpret(const std::vector<std::unique_ptr<ast::statement>>& statements_) {
    try {
        for (const auto& stmt : statements_) {
            execute(*stmt);
        }
    } catch (const std::runtime_error& e) {
        reporter_.error(0, 0, e.what());
    }
}

void interpreter::execute(const ast::statement& stmt) {
    std::visit(core::overloaded{
        [this](const ast::expression_stmt& s) { execute_expression_stmt(s); },
        [this](const ast::var_declaration& s) { execute_var_declaration(s); },
        [this](const ast::block_stmt& s) { execute_block(s); },
        [this](const ast::while_stmt& s) { execute_while(s); },
        [this](const ast::if_stmt& s) { execute_if(s); },
        }, stmt.data_);
}

void interpreter::execute_expression_stmt(const ast::expression_stmt& stmt) {
    evaluate(*stmt.expr_);
}

void interpreter::execute_var_declaration(const ast::var_declaration& stmt) {
    std::string name_{ stmt.name_.lexeme_ };

    value init_val = [&]() {
        switch (stmt.type_) {
        case core::value_type::INT:    return value(0);
        case core::value_type::DOUBLE: return value(0.0);
        case core::value_type::BOOL:   return value(false);
        case core::value_type::STRING: return value(std::string(""));
        case core::value_type::VOID:   return value();
        default: throw std::runtime_error("Unknown variable type");
        }
        }();

    if (stmt.initializer_ != nullptr) {
        value init = evaluate(*stmt.initializer_);
        if (init.type() != stmt.type_) {
            throw std::runtime_error("Type mismatch in variable initialisation of '" + name_ + "'");
        }
        init_val = std::move(init);
    }

    current_env_->define(name_, std::move(init_val));
}

void interpreter::execute_block(const ast::block_stmt& stmt) {
    current_env_->push_scope();
    for (const auto& s : stmt.statements_) {
        execute(*s);
    }
    current_env_->pop_scope();
}

void interpreter::execute_while(const ast::while_stmt& stmt) {
    while (true) {
        value cond = evaluate(*stmt.condition_);
        if (!cond.as_bool().value_or(false)) break;
        execute(*stmt.body_);
    }
}

void interpreter::execute_if(const ast::if_stmt& stmt) {
    value cond = evaluate(*stmt.condition_);
    if (cond.as_bool().value_or(false)) {
        execute(*stmt.then_branch_);
    }
    else if (stmt.else_branch_ != nullptr) {
        execute(*stmt.else_branch_);
    }
}

value interpreter::evaluate(const ast::expression& expr_) {
    return std::visit(core::overloaded{
        [this](const ast::literal_expr& e) { return evaluate_literal(e); },
        [this](const ast::variable_expr& e) { return evaluate_variable(e); },
        [this](const ast::binary_expr& e) { return evaluate_binary(e); },
        [this](const ast::unary_expr& e) { return evaluate_unary(e); },
        }, expr_.data_);
}

value interpreter::evaluate_literal(const ast::literal_expr& expr_) {
    const auto& token = expr_.value_;
    switch (token.type_) {
    case core::token_type::NUMBER: {
        std::string_view lex = token.lexeme_;
        bool is_double = (lex.find('.') != std::string_view::npos ||
            lex.find('e') != std::string_view::npos ||
            lex.find('E') != std::string_view::npos);
        if (is_double) {
            return value(std::stod(std::string(lex)));
        } else {
            return value(std::stoi(std::string(lex)));
        }
    }
    case core::token_type::TRUE:
        return value(true);
    case core::token_type::FALSE:
        return value(false);
    case core::token_type::STRING:
        return value(std::string(token.lexeme_));
    default:
        throw std::runtime_error("Unexpected literal");
    }
}

value interpreter::evaluate_variable(const ast::variable_expr& expr_) {
    std::string name_{ expr_.name_.lexeme_ };
    auto val = current_env_->get(name_);
    if (!val) {
        throw std::runtime_error("Undefined variable '" + name_ + "'");
    }
    return *val;
}

value interpreter::evaluate_binary(const ast::binary_expr& expr_) {
    value left = evaluate(*expr_.left_);
    value right = evaluate(*expr_.right_);

    switch (expr_.op_.type_) {
    case core::token_type::PLUS:         return left.add(right);
    case core::token_type::MINUS:        return left.sub(right);
    case core::token_type::STAR:         return left.mul(right);
    case core::token_type::SLASH:        return left.div(right);
    case core::token_type::PERCENT:      return left.mod(right);
    case core::token_type::EQUAL_EQUAL:  return left.eq(right);
    case core::token_type::BANG_EQUAL:   return left.neq(right);
    case core::token_type::LESS:         return left.lt(right);
    case core::token_type::LESS_EQUAL:   return left.le(right);
    case core::token_type::GREATER:      return left.gt(right);
    case core::token_type::GREATER_EQUAL:return left.ge(right);
    case core::token_type::AND:          return left.and_op(right);
    case core::token_type::OR:           return left.or_op(right);
    default:
        throw std::runtime_error("Unsupported binary operator");
    }
}

value interpreter::evaluate_unary(const ast::unary_expr& expr_) {
    value operand = evaluate(*expr_.operand_);

    switch (expr_.op_.type_) {
    case core::token_type::MINUS: {
        if (operand.type() == core::value_type::INT) {
            return value(-operand.as_int().value());
        } else if (operand.type() == core::value_type::DOUBLE) {
            return value(-operand.as_double().value());
        }
        throw std::runtime_error("Unary minus requires numeric operand");
    }
    case core::token_type::BANG: {
        return operand.not_op();
    }
    default:
        throw std::runtime_error("Unsupported unary operator");
    }
}

}