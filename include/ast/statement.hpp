#pragma once

#include "ast/expression.hpp"
#include "core/token.hpp"
#include "core/type.hpp"
#include <vector>

namespace ast {

class expression_stmt : public statement {
public:
    expression_stmt(std::unique_ptr<expression> expr);
    std::unique_ptr<expression> expr_;
};

class var_declaration : public statement {
public:
    var_declaration(core::value_type type, core::token name, std::unique_ptr<expression> initializer);

    core::value_type type_;
    core::token name_;
    std::unique_ptr<expression> initializer_;
};

class block_stmt : public statement {
public:
    block_stmt(std::vector<std::unique_ptr<statement>> statements);

    std::vector<std::unique_ptr<statement>> statements_;
};

class while_stmt : public statement {
public:
    while_stmt(std::unique_ptr<expression> condition, std::unique_ptr<statement> body);

    std::unique_ptr<expression> condition_;
    std::unique_ptr<statement> body_;
};

} 