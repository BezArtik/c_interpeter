#include "ast/expression.hpp"
#include "core/token.hpp"
#include <memory>

namespace ast {

expression::expression() : data_(literal_expr{}) {}
expression::expression(literal_expr e) : data_(std::move(e)) {}
expression::expression(variable_expr e) : data_(std::move(e)) {}
expression::expression(binary_expr e) : data_(std::move(e)) {}
expression::expression(unary_expr e) : data_(std::move(e)) {}
expression::expression(postfix_expr e) : data_(std::move(e)) {}
expression::expression(call_expr e) : data_(std::move(e)) {}

}