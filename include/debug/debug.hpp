// include/debug/debug.hpp


#pragma once
#include "ast/expression.hpp"
#include "ast/statement.hpp"
#include "core/token/token.hpp"
#include <vector>
#include <memory>
#include <string>


namespace runtime { class value; }

namespace debug {

void print_tokens(const std::vector<core::token>& tokens);
const char* type_name(const core::type& t);

void print_ast(const std::vector<std::unique_ptr<ast::statement>>& statements);
void print_statement(const ast::statement& stmt, int level = 0);
void print_expression(const ast::expression& expr, int level = 0);

void print_semantic_info(const std::vector<std::unique_ptr<ast::statement>>& statements);

void print_value(const runtime::value& val, int indent = 0);
void print_execution(const std::string& message, int indent = 0);

} // namespace debug