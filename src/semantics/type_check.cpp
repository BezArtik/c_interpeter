#include "semantics/type_check.hpp"
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "core/overloaded.hpp"
#include <string>

namespace semantics {

type_checker::type_checker(core::error_reporter& reporter)
    : reporter_(reporter) {
    register_builtins();
}

bool type_checker::check(const std::vector<std::unique_ptr<ast::statement>>& statements) {
    for (const auto& stmt : statements) {
        check_statement(*stmt);
    }
    return !reporter_.has_error();
}

void type_checker::register_builtins() {
    add_builtin("print", core::value_type::VOID, { core::value_type::INT });
    add_builtin("print", core::value_type::VOID, { core::value_type::DOUBLE });
    add_builtin("print", core::value_type::VOID, { core::value_type::BOOL });
    add_builtin("print", core::value_type::VOID, { core::value_type::STRING });
}

void type_checker::add_builtin(const std::string& name, core::value_type return_type,
    const std::vector<core::value_type>& param_types) {
    builtins_[name].push_back(param_types);
}

bool type_checker::is_assignable(core::value_type target, core::value_type source) noexcept {
    if (target == source) return true;
    if (target == core::value_type::DOUBLE && source == core::value_type::INT) return true;
    return false;
}

void type_checker::check_statement(const ast::statement& stmt) {
    std::visit(core::overloaded{
        [this](const ast::expression_stmt& s) { check_expression_stmt(s); },
        [this](const ast::var_declaration& s) { check_var_declaration(s); },
        [this](const ast::block_stmt& s) { check_block(s); },
        [this](const ast::while_stmt& s) { check_while(s); },
		[this](const ast::for_stmt& s) { check_for(s); },
        [this](const ast::if_stmt& s) { check_if(s); },
        [this](const ast::return_stmt& s) { check_return_stmt(s); },
        [this](const ast::func_declaration& s) { check_func_declaration(s); },
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

    if (stmt.initializer_) {
        core::value_type init_type = type_of(*stmt.initializer_);
        if (init_type == core::value_type::UNKNOWN) {
            return;
        }
        if (!is_assignable(stmt.type_, init_type)) {
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

void type_checker::check_for(const ast::for_stmt& stmt) {
	symbols_.push_scope();
	if (stmt.initializer_) {
		check_statement(*stmt.initializer_);
	}
	if (stmt.condition_) {
		core::value_type cond_type = type_of(*stmt.condition_);
		if (cond_type != core::value_type::BOOL && cond_type != core::value_type::UNKNOWN) {
			reporter_.error(0, 0, "for condition must be a boolean expression");
		}
	}
	if (stmt.increment_) {
		type_of(*stmt.increment_);
	}
	check_statement(*stmt.body_);
	symbols_.pop_scope();
}

void type_checker::check_if(const ast::if_stmt& stmt) {
    core::value_type cond_type = type_of(*stmt.condition_);
    if (cond_type != core::value_type::BOOL && cond_type != core::value_type::UNKNOWN) {
        reporter_.error(0, 0, "if condition must be a boolean expression");
    }
    check_statement(*stmt.then_branch_);
    if (stmt.else_branch_) {
        check_statement(*stmt.else_branch_);
    }
}

void type_checker::check_return_stmt(const ast::return_stmt& stmt) {
    if (!curr_return_type_) {
        reporter_.error(stmt.keyword_.line_, stmt.keyword_.column_,
            "return statement outside of function");
        return;
    }

    if (!stmt.value_) {
        if (*curr_return_type_ != core::value_type::VOID) {
            reporter_.error(stmt.keyword_.line_, stmt.keyword_.column_,
                "return with no value in non-void function");
        }
        return;
    }

    core::value_type return_type = type_of(*stmt.value_);
    if (return_type == core::value_type::UNKNOWN) {
        return;
    }
    if (return_type != *curr_return_type_) {
        reporter_.error(stmt.keyword_.line_, stmt.keyword_.column_,
            "return type mismatch");
    }
}

void type_checker::check_func_declaration(const ast::func_declaration& stmt) {
    std::string name{ stmt.name_.lexeme_ };

    if (symbols_.defined_locally(name)) {
        reporter_.error(stmt.name_.line_, stmt.name_.column_,
            "redeclaration of function '" + name + "'");
        return;
    }

    std::vector<core::value_type> param_types;
    for (const auto& param : stmt.params_) {
        param_types.push_back(param.type_);
    }

    symbols_.define_function(name, stmt.return_type_, param_types);

    symbols_.push_scope();

    for (const auto& param : stmt.params_) {
        std::string param_name{ param.name_.lexeme_ };
        symbols_.define(param_name, param.type_);
        symbols_.mark_initialized(param_name);
    }

    auto prev_return_type = curr_return_type_;
    curr_return_type_ = stmt.return_type_;

    for (const auto& s : stmt.body_->statements_) {
        check_statement(*s);
    }

    curr_return_type_ = prev_return_type;
    symbols_.pop_scope();
}

core::value_type type_checker::type_of(const ast::expression& expr) {
    return std::visit(core::overloaded{
        [this](const ast::literal_expr& e) { return type_of_literal(e); },
        [this](const ast::variable_expr& e) { return type_of_variable(e); },
        [this](const ast::binary_expr& e) { return type_of_binary(e); },
        [this](const ast::unary_expr& e) { return type_of_unary(e); },
		[this](const ast::postfix_expr& e) { return type_of_postfix(e); },
        [this](const ast::call_expr& e) { return type_of_call(e); },
        }, expr.data_);
}

core::value_type type_checker::type_of_literal(const ast::literal_expr& expr) {
    const auto& token = expr.value_;
    switch (token.type_) {
    case core::token_type::NUMBER: {
        std::string_view lex = token.lexeme_;
        bool is_double = (lex.find('.') != std::string_view::npos);
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

core::value_type type_checker::type_of_binary(const ast::binary_expr& expr) {
    core::value_type left = type_of(*expr.left_);
    core::value_type right = type_of(*expr.right_);
    if (left == core::value_type::UNKNOWN || right == core::value_type::UNKNOWN) {
        return core::value_type::UNKNOWN;
    }

    auto is_numeric = [](core::value_type t) {
        return t == core::value_type::INT || t == core::value_type::DOUBLE;
        };

    core::token_type op = expr.op_.type_;

    if (op == core::token_type::EQUAL) {
        if (!is_assignable(left, right)) {
            reporter_.error(expr.op_.line_, expr.op_.column_,
                "type mismatch in assignment");
            return core::value_type::UNKNOWN;
        }
        return left;
    }

    if (op == core::token_type::PLUS || op == core::token_type::MINUS ||
        op == core::token_type::STAR || op == core::token_type::SLASH ||
        op == core::token_type::PERCENT) {
        if (!is_numeric(left) || !is_numeric(right)) {
            reporter_.error(expr.op_.line_, expr.op_.column_,
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
            reporter_.error(expr.op_.line_, expr.op_.column_,
                "comparison requires numeric operands");
            return core::value_type::UNKNOWN;
        }
        return core::value_type::BOOL;
    }

    if (op == core::token_type::AND || op == core::token_type::OR) {
        if (left != core::value_type::BOOL || right != core::value_type::BOOL) {
            reporter_.error(expr.op_.line_, expr.op_.column_,
                "logical operators require boolean operands");
            return core::value_type::UNKNOWN;
        }
        return core::value_type::BOOL;
    }

    reporter_.error(expr.op_.line_, expr.op_.column_, "unsupported binary operator");
    return core::value_type::UNKNOWN;
}

bool type_checker::is_lvalue(const ast::expression& expr) {
    return std::holds_alternative<ast::variable_expr>(expr.data_);
}

core::value_type type_checker::type_of_unary(const ast::unary_expr& expr) {
    core::value_type operand_type = type_of(*expr.operand_);
    if (operand_type == core::value_type::UNKNOWN) {
        return core::value_type::UNKNOWN;
    }

    core::token_type op = expr.op_.type_;

    if (op == core::token_type::MINUS) {
        if (operand_type != core::value_type::INT && operand_type != core::value_type::DOUBLE) {
            reporter_.error(expr.op_.line_, expr.op_.column_,
                "unary minus requires numeric operand");
            return core::value_type::UNKNOWN;
        }
        return operand_type;
    }

	if (op == core::token_type::INCREMENT || op == core::token_type::DECREMENT) {
		if (operand_type != core::value_type::INT && operand_type != core::value_type::DOUBLE) {
			reporter_.error(expr.op_.line_, expr.op_.column_,
				"increment/decrement requires numeric operand");
			return core::value_type::UNKNOWN;
		}
		if (!is_lvalue(*expr.operand_)) {
			reporter_.error(expr.op_.line_, expr.op_.column_,
				"increment/decrement requires an lvalue operand");
			return core::value_type::UNKNOWN;
		}
		return operand_type;
	}

    if (op == core::token_type::BANG) {
        if (operand_type != core::value_type::BOOL) {
            reporter_.error(expr.op_.line_, expr.op_.column_,
                "logical not requires boolean operand");
            return core::value_type::UNKNOWN;
        }
        return core::value_type::BOOL;
    }

    reporter_.error(expr.op_.line_, expr.op_.column_, "unsupported unary operator");
    return core::value_type::UNKNOWN;
}

core::value_type type_checker::type_of_postfix(const ast::postfix_expr& expr) {
    core::value_type operand_type = type_of(*expr.operand_);
    if (operand_type == core::value_type::UNKNOWN) {
        return core::value_type::UNKNOWN;
    }
    if (operand_type != core::value_type::INT && operand_type != core::value_type::DOUBLE) {
        reporter_.error(expr.op_.line_, expr.op_.column_,
            "increment/decrement requires numeric operand");
        return core::value_type::UNKNOWN;
    }
    if (!is_lvalue(*expr.operand_)) {
        reporter_.error(expr.op_.line_, expr.op_.column_,
            "increment/decrement requires variable");
        return core::value_type::UNKNOWN;
    }
    return operand_type;
}

core::value_type type_checker::type_of_call(const ast::call_expr& expr) {
    std::string name{ expr.callee_.lexeme_ };
    auto builtin_it = builtins_.find(name);
    if (builtin_it != builtins_.end()) {
        for (const auto& param_types : builtin_it->second) {
            if (expr.args_.size() == param_types.size()) {
                bool match = true;
                for (size_t i = 0; i < expr.args_.size(); i++) {
                    core::value_type arg_type = type_of(*expr.args_[i]);
                    if (arg_type != param_types[i]) {
                        match = false;
                        break;
                    }
                }
                if (match) {
                    return core::value_type::VOID; 
                }
            }
        }
        reporter_.error(expr.callee_.line_, expr.callee_.column_,
            "no matching overload for builtin '" + name + "'");
        return core::value_type::UNKNOWN;
    }

    auto info = symbols_.lookup(name);

    if (!info) {
        reporter_.error(expr.callee_.line_, expr.callee_.column_,
            "undefined function '" + name + "'");
        return core::value_type::UNKNOWN;
    }

    if (info->kind_ != symbol_kind::FUNCTION) {
        reporter_.error(expr.callee_.line_, expr.callee_.column_,
            "'" + name + "' is not a function");
        return core::value_type::UNKNOWN;
    }

    if (expr.args_.size() != info->param_types_.size()) {
        reporter_.error(expr.callee_.line_, expr.callee_.column_,
            "function '" + name + "' expects " +
            std::to_string(info->param_types_.size()) +
            " arguments, got " + std::to_string(expr.args_.size()));
        return core::value_type::UNKNOWN;
    }

    for (size_t i = 0; i < expr.args_.size(); i++) {
        core::value_type arg_type = type_of(*expr.args_[i]);
        if (arg_type == core::value_type::UNKNOWN) {
            return core::value_type::UNKNOWN;
        }
        if (arg_type != info->param_types_[i]) {
            reporter_.error(expr.callee_.line_, expr.callee_.column_,
                "argument " + std::to_string(i + 1) + " type mismatch in call to '" + name + "'");
            return core::value_type::UNKNOWN;
        }
    }

    return info->type_;
}

}