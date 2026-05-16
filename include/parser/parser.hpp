#pragma once
#include "ast/ast_node.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "core/token.hpp"
#include "core/error_report.hpp"
#include <vector>
#include <memory>
#include <string_view>

namespace parser {

class parser {
    using ptr_statement = std::unique_ptr<ast::statement>;
    using ptr_expression = std::unique_ptr<ast::expression>;
public:
    parser(const std::vector<core::token>& tokens, core::error_reporter& reporter);

    std::vector<ptr_statement> parse();

private:

    const core::token& advance();
    const core::token& peek() const;
    const core::token& prev() const;
    bool is_at_end() const;
    bool check(core::token_type type) const;
    bool match(std::initializer_list<core::token_type> types);
    const core::token& consume(core::token_type type, std::string_view msg);

    ptr_statement declaration();
    ptr_statement statement();
    ptr_statement while_statement();
    ptr_statement block_statement();
    ptr_statement var_declaration(core::value_type type);

    ptr_expression term();
    ptr_expression factor();
    ptr_expression expression();
    ptr_expression equality();
    ptr_expression primary();

    void synchronize();

    const std::vector<core::token>& tokens_;
    core::error_reporter& reporter_;
    size_t current_ = 0;
};

}