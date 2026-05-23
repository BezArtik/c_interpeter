#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include <vector>
#include <memory>

namespace ast {

statement::statement() : data_(expression_stmt{}) {}
statement::statement(expression_stmt s) : data_(std::move(s)) {}
statement::statement(var_declaration s) : data_(std::move(s)) {}
statement::statement(block_stmt s) : data_(std::move(s)) {}
statement::statement(while_stmt s) : data_(std::move(s)) {}
statement::statement(for_stmt s) : data_(std::move(s)) {}
statement::statement(if_stmt s) : data_(std::move(s)) {}
statement::statement(return_stmt s) : data_(std::move(s)) {}
statement::statement(func_declaration s) : data_(std::move(s)) {}

}