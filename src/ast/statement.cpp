#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include <vector>
#include <memory>

namespace ast {

expression_stmt::expression_stmt(std::unique_ptr<expression> expr)
        : expr_(std::move(expr)) {}


var_declaration::var_declaration(core::value_type type, core::token name, 
                                 std::unique_ptr<expression> initializer)
        : type_(type), name_(name), initializer_(std::move(initializer)) {}

block_stmt::block_stmt(std::vector<std::unique_ptr<statement>> statements)
    : statements_(std::move(statements)) {}

while_stmt::while_stmt(std::unique_ptr<expression> condition, 
                       std::unique_ptr<statement> body)
        : condition_(std::move(condition)), body_(std::move(body)) {
    }

}