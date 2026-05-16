#pragma once

#include <memory>
#include <vector>
#include "runtime/environment.hpp"
#include "runtime/value.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "core/error_report.hpp"

namespace runtime {

class interpreter {
public:
    interpreter(core::error_reporter& reporter);

    void interpret(const std::vector<std::unique_ptr<ast::statement>>& statements);

private:
    core::error_reporter& reporter_;
    std::unique_ptr<environment> global_env_; 
    environment* current_env_;           

    void execute(const ast::statement& stmt);
    void execute_expression_stmt(const ast::expression_stmt& stmt);
    void execute_var_declaration(const ast::var_declaration& stmt);
    void execute_block(const ast::block_stmt& stmt);
    void execute_while(const ast::while_stmt& stmt);

    value evaluate(const ast::expression& expr);
    value evaluate_literal(const ast::literal_expr& expr);
    value evaluate_variable(const ast::variable_expr& expr);
    value evaluate_binary(const ast::binary_expr& expr);
};

}