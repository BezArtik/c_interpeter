#include "runtime/interpreter.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include <stdexcept>
#include <string>

namespace runtime {

interpreter::interpreter(core::error_reporter& reporter)
    : reporter_(reporter)
    , global_env_(std::make_unique<environment>())
    , current_env_(global_env_.get()) {
}

void interpreter::interpret(const std::vector<std::unique_ptr<ast::statement>>& statements) {
    try {
        for (const auto& stmt : statements) {
            execute(*stmt);
        }
    } catch (const std::runtime_error& e) {
        reporter_.error(0, 0, e.what());
    }
}

void interpreter::execute(const ast::statement& stmt) {
    if (auto* p = dynamic_cast<const ast::expression_stmt*>(&stmt)) {
        execute_expression_stmt(*p);
    } else if (auto* p = dynamic_cast<const ast::var_declaration*>(&stmt)) {
        execute_var_declaration(*p);
    } else if (auto* p = dynamic_cast<const ast::block_stmt*>(&stmt)) {
        execute_block(*p);
    } else if (auto* p = dynamic_cast<const ast::while_stmt*>(&stmt)) {
        execute_while(*p);
    }
    else {
        throw std::runtime_error("Unknown statement type");
    }
}

void interpreter::execute_expression_stmt(const ast::expression_stmt& stmt) {
    evaluate(*stmt.expr_); 
}

void interpreter::execute_var_declaration(const ast::var_declaration& stmt) {
    std::string name{ stmt.name_.lexeme_ };

    value init_val = [&]() {
        switch (stmt.type_) {
        case core::value_type::INT:    return value(0);
        case core::value_type::DOUBLE: return value(0.0);
        case core::value_type::BOOL:   return value(false);
        case core::value_type::STRING: return value("");
        case core::value_type::VOID:   return value();
        default: throw std::runtime_error("Unknown variable type");
        }}();

    if (stmt.initializer_ != nullptr) {
        value init = evaluate(*stmt.initializer_);
        if (init.type() != stmt.type_) {
            throw std::runtime_error("Type mismatch in variable initialisation of '" + name + "'");
        }
        init_val = std::move(init);
    }

    current_env_->define(name, std::move(init_val));
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

value interpreter::evaluate(const ast::expression& expr) {
    if (auto* p = dynamic_cast<const ast::literal_expr*>(&expr)) {
        return evaluate_literal(*p);
    } else if (auto* p = dynamic_cast<const ast::variable_expr*>(&expr)) {
        return evaluate_variable(*p);
    } else if (auto* p = dynamic_cast<const ast::binary_expr*>(&expr)) {
        return evaluate_binary(*p);
    }
    throw std::runtime_error("Unknown expression type");
}

value interpreter::evaluate_literal(const ast::literal_expr& expr) {
    const auto& token = expr.value_;
    switch (token.type_) {
    case core::token_type::NUMBER: {
        std::string_view lex = token.lexeme_;
        bool is_double = 
            (lex.find('.') != std::string_view::npos ||
            lex.find('e') != std::string_view::npos ||
            lex.find('E') != std::string_view::npos);
        if (is_double) {
            double d = std::stod(std::string(lex));
            return value(d);
        }
        else {
            int i = std::stoi(std::string(lex));
            return value(i);
        }
    }
    case core::token_type::TRUE:
        return value(true);
    case core::token_type::FALSE:
        return value(false);
    case core::token_type::STRING: {
        std::string s{ token.lexeme_ };
        return value(s);
    }
    default:
        throw std::runtime_error("Unexpected literal");
    }
}

value interpreter::evaluate_variable(const ast::variable_expr& expr) {
    std::string name{ expr.name_.lexeme_ };
    auto val = current_env_->get(name);
    if (!val) {
        throw std::runtime_error("Undefined variable '" + name + "'");
    }
    return *val;
}

value interpreter::evaluate_binary(const ast::binary_expr& expr) {
    value left = evaluate(*expr.left_);
    value right = evaluate(*expr.right_);

    switch (expr.operator_.type_) {
    case core::token_type::PLUS:         return left.add(right);
    case core::token_type::MINUS:        return left.sub(right);
    case core::token_type::STAR:         return left.mul(right);
    case core::token_type::SLASH:        return left.div(right);
    case core::token_type::EQUAL_EQUAL:  return left.eq(right);
    case core::token_type::BANG_EQUAL:   return left.neq(right);
    case core::token_type::LESS:         return left.lt(right);
    case core::token_type::LESS_EQUAL:   return left.le(right);
    case core::token_type::GREATER:      return left.gt(right);
    case core::token_type::GREATER_EQUAL:return left.ge(right);
    default:
        throw std::runtime_error("Unsupported binary operator");
    }
}

}