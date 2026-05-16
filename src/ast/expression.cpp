#include "ast/expression.hpp"

#include "core/token.hpp"
#include <memory>

namespace ast {

literal_expr::literal_expr(core::token value) : value_(value) {}

binary_expr::binary_expr(std::unique_ptr<expression> left, 
                         core::token op, 
                         std::unique_ptr<expression> right)
        : left_(std::move(left)), operator_(op), right_(std::move(right)) {}

variable_expr::variable_expr(core::token name) : name_(name) {}

}