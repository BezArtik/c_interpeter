#pragma once

#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "core/token.hpp"
#include "core/error_report.hpp"
#include "core/type.hpp"
#include <vector>
#include <memory>
#include <string_view>

namespace parser {

class parser {
public:
    parser(const std::vector<core::token>& tokens, core::error_reporter& reporter);

    std::vector<std::unique_ptr<ast::statement>> parse();

private:
    const core::token& advance();
    const core::token& peek() const;
    const core::token& prev() const;
    bool is_at_end() const;
    bool check(core::token_type type) const;
    bool match(std::initializer_list<core::token_type> types);
    const core::token& consume(core::token_type type, std::string_view msg);

    std::unique_ptr<ast::statement> declaration();
    std::unique_ptr<ast::statement> statement();
    std::unique_ptr<ast::statement> var_declaration(core::value_type type);
    std::unique_ptr<ast::statement> while_statement();
    std::unique_ptr<ast::statement> if_statement();
    std::unique_ptr<ast::statement> block_statement();

    std::unique_ptr<ast::expression> expression();
    std::unique_ptr<ast::expression> equality();
    std::unique_ptr<ast::expression> comparison();
    std::unique_ptr<ast::expression> term();
    std::unique_ptr<ast::expression> factor();
    std::unique_ptr<ast::expression> unary();
    std::unique_ptr<ast::expression> primary();

    void synchronize();

    const std::vector<core::token>& tokens_;
    core::error_reporter& reporter_;
    size_t current_ = 0;
};

} 