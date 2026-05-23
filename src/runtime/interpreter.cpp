#include "runtime/interpreter.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "core/overloaded.hpp"
#include "semantics/type_check.hpp"
#include <stdexcept>
#include <string>
#include <iostream>

namespace runtime {

interpreter::interpreter(core::error_reporter& reporter)
    : reporter_(reporter)
    , global_env_(std::make_unique<environment>())
    , current_env_(global_env_.get()) {

    global_env_->define_builtin("print", [](const std::vector<value>& args) {
        for (size_t i = 0; i < args.size(); i++) {
            if (i > 0) std::cout << " ";
            std::cout << args[i].to_string();
        }
        std::cout << std::endl;
        return value();
        });
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
    std::visit(core::overloaded{
        [this](const ast::expression_stmt& s) { execute_expression_stmt(s); },
        [this](const ast::var_declaration& s) { execute_var_declaration(s); },
        [this](const ast::block_stmt& s) { execute_block(s); },
        [this](const ast::while_stmt& s) { execute_while(s); },
		[this](const ast::for_stmt& s) { execute_for(s); },
        [this](const ast::if_stmt& s) { execute_if(s); },
        [this](const ast::return_stmt& s) { execute_return_stmt(s); },
        [this](const ast::func_declaration& s) { execute_func_declaration(s); },
        }, stmt.data_);
}

void interpreter::execute_expression_stmt(const ast::expression_stmt& stmt) {
    evaluate(*stmt.expr_);
}

void interpreter::execute_var_declaration(const ast::var_declaration& stmt) {
    std::string name_{ stmt.name_.lexeme_ };

    auto init_val = [&]() {
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
        auto init = evaluate(*stmt.initializer_);
        auto init_type = init.type();
        auto target_type = stmt.type_;

        if (init_type == core::value_type::INT && target_type == core::value_type::DOUBLE) {
            init_val = value(static_cast<double>(init.as_int().value()));
        } else if (init_type != target_type) {
            throw std::runtime_error("Type mismatch in variable initialisation of '" + name_ + "'");
        } else {
            init_val = std::move(init);
        }
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

void interpreter::execute_for(const ast::for_stmt& stmt) {
	current_env_->push_scope();
	if (stmt.initializer_) {
		execute(*stmt.initializer_);
	}
	while (true) {
		if (stmt.condition_) {
			auto cond = evaluate(*stmt.condition_);
			if (!cond.as_bool().value_or(false)) break;
		}
		execute(*stmt.body_);
		if (stmt.increment_) {
			evaluate(*stmt.increment_);
		}
	}
	current_env_->pop_scope();
}

void interpreter::execute_if(const ast::if_stmt& stmt) {
    auto cond = evaluate(*stmt.condition_);
    if (cond.as_bool().value_or(false)) {
        execute(*stmt.then_branch_);
    } else if (stmt.else_branch_) {
        execute(*stmt.else_branch_);
    }
}

void interpreter::execute_return_stmt(const ast::return_stmt& stmt) {
    value ret_val;
    if (stmt.value_) {
        ret_val = evaluate(*stmt.value_);
    } else {
        ret_val = value();
    }
    throw return_exception{ std::move(ret_val) };
}

void interpreter::execute_func_declaration(const ast::func_declaration& stmt) {
    std::string name{ stmt.name_.lexeme_ };
    functions_[name] = &stmt;
}

value interpreter::evaluate(const ast::expression& expr) {
    return std::visit(core::overloaded{
        [this](const ast::literal_expr& e) { return evaluate_literal(e); },
        [this](const ast::variable_expr& e) { return evaluate_variable(e); },
        [this](const ast::binary_expr& e) { return evaluate_binary(e); },
        [this](const ast::unary_expr& e) { return evaluate_unary(e); },
		[this](const ast::postfix_expr& e) { return evaluate_postfix(e); },
        [this](const ast::call_expr& e) { return evaluate_call(e); },
        }, expr.data_);
}

value interpreter::evaluate_literal(const ast::literal_expr& expr) {
    const auto& token = expr.value_;
    switch (token.type_) {
    case core::token_type::NUMBER: {
        std::string_view lex = token.lexeme_;
        if (core::is_double_literal(lex)) {
            return value(std::stod(std::string(lex)));
        } else {
            return value(static_cast<int64_t>(std::stoll(std::string(lex))));
        }
    }
    case core::token_type::TRUE:
        return value(true);
    case core::token_type::FALSE:
        return value(false);
    case core::token_type::STRING: {
        std::string_view lex = token.lexeme_;
        std::string s{ lex.substr(1, lex.size() - 2) };
        return value(std::move(s));
    }
    default:
        throw std::runtime_error("Unexpected literal");
    }
}

value interpreter::evaluate_variable(const ast::variable_expr& expr) {
    std::string name_{ expr.name_.lexeme_ };
    auto val = current_env_->get(name_);
    if (!val) {
        throw std::runtime_error("Undefined variable '" + name_ + "'");
    }
    return *val;
}

value interpreter::evaluate_binary(const ast::binary_expr& expr) {
    if (expr.op_.type_ == core::token_type::EQUAL) {
        const auto* var = std::get_if<ast::variable_expr>(&expr.left_->data_);
        if (!var) {
            throw std::runtime_error("Invalid assignment target");
        }

        std::string name{ var->name_.lexeme_ };
        auto right = evaluate(*expr.right_);
        current_env_->assign(name, right);
        return right;
    }

    if (expr.op_.type_ == core::token_type::AND) {
        auto left = evaluate(*expr.left_);
        if (!left.as_bool().value_or(false)) {
            return value(false);
        }
        auto right = evaluate(*expr.right_);
        return value(right.as_bool().value_or(false));
    }

    if (expr.op_.type_ == core::token_type::OR) {
        auto left = evaluate(*expr.left_);
        if (left.as_bool().value_or(false)) {
            return value(true);
        }
        auto right = evaluate(*expr.right_);
        return value(right.as_bool().value_or(false));
    }

    auto left = evaluate(*expr.left_);
    auto right = evaluate(*expr.right_);

    switch (expr.op_.type_) {
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

value interpreter::evaluate_unary(const ast::unary_expr& expr) {
    value operand = evaluate(*expr.operand_);

    switch (expr.op_.type_) {
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
    case core::token_type::INCREMENT:
    case core::token_type::DECREMENT: {
        const auto& var = std::get<ast::variable_expr>(expr.operand_->data_);
        std::string name{ var.name_.lexeme_ };
        auto old_val = evaluate_variable(var);
        value new_val;
        if (old_val.type() == core::value_type::INT) {
            auto v = old_val.as_int().value();
            new_val = value(expr.op_.type_ == core::token_type::INCREMENT ? v + 1 : v - 1);
        } else {
            auto v = old_val.as_double().value();
            new_val = value(expr.op_.type_ == core::token_type::INCREMENT ? v + 1.0 : v - 1.0);
        }
        current_env_->assign(name, new_val);
        return new_val;
    }
    default:
        throw std::runtime_error("Unsupported unary operator");
    }
}

value interpreter::evaluate_postfix(const ast::postfix_expr& expr) {
    const auto& var = std::get<ast::variable_expr>(expr.operand_->data_);
    std::string name{ var.name_.lexeme_ };
    auto old_val = evaluate_variable(var);
    value new_val;
    if (old_val.type() == core::value_type::INT) {
        int64_t v = old_val.as_int().value();
        new_val = value(expr.op_.type_ == core::token_type::INCREMENT ? v + 1 : v - 1);
    } else {
        double v = old_val.as_double().value();
        new_val = value(expr.op_.type_ == core::token_type::INCREMENT ? v + 1.0 : v - 1.0);
    }
    current_env_->assign(name, new_val);
    return old_val;
}

value interpreter::evaluate_call(const ast::call_expr& expr) {
    std::string name{ expr.callee_.lexeme_ };

    auto builtin = current_env_->get_builtin(name);
    if (builtin) {
        std::vector<value> args;
        for (const auto& arg : expr.args_) {
            args.push_back(evaluate(*arg));
        }
        return (*builtin)(args);
    }

    auto func_it = functions_.find(name);
    if (func_it == functions_.end()) {
        throw std::runtime_error("Undefined function '" + name + "'");
    }

    const ast::func_declaration& func = *func_it->second;

    std::vector<value> args;
    for (const auto& arg : expr.args_) {
        args.push_back(evaluate(*arg));
    }

    current_env_->push_scope();

    for (size_t i = 0; i < func.params_.size(); i++) {
        std::string param_name{ func.params_[i].name_.lexeme_ };
        current_env_->define(param_name, std::move(args[i]));
    }

    auto* prev_env = current_env_;

    value result;
    try {
        for (const auto& s : func.body_->statements_) {
            execute(*s);
        }
        switch (func.return_type_) {
        case core::value_type::INT:    result = value(0); break;
        case core::value_type::DOUBLE: result = value(0.0); break;
        case core::value_type::BOOL:   result = value(false); break;
        case core::value_type::STRING: result = value(std::string("")); break;
        default: result = value();
        }
    } catch (return_exception& ret) {
        result = std::move(ret.return_value_);
    }

    current_env_ = prev_env;
    current_env_->pop_scope();

    return result;
}

}