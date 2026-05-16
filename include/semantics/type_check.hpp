#pragma once

#include <memory>
#include <vector>
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "core/error_report.hpp"
#include "semantics/symbol_table.hpp"

namespace semantics {

class type_checker {
public:
    type_checker(core::error_reporter& reporter);

    bool check(const std::vector<std::unique_ptr<ast::statement>>& statements);

private:
    core::error_reporter& reporter_;
    symbol_table symbols_;

    void check_statement(const ast::statement& stmt);
    void check_expression_stmt(const ast::expression_stmt& stmt);
    void check_var_declaration(const ast::var_declaration& stmt);
    void check_block(const ast::block_stmt& stmt);
    void check_while(const ast::while_stmt& stmt);

    core::value_type type_of(const ast::expression& expr);
    core::value_type type_of_literal(const ast::literal_expr& expr);
    core::value_type type_of_variable(const ast::variable_expr& expr);
    core::value_type type_of_binary(const ast::binary_expr& expr);
};

} 