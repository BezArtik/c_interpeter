#pragma once

#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "core/token.hpp"
#include "core/error_report.hpp"
#include "core/type.hpp"
#include <vector>
#include <memory>
#include <string_view>
#include <functional>

namespace parser {

class parser {
    using stmt_ptr = std::unique_ptr<ast::statement>;
    using expr_ptr = std::unique_ptr<ast::expression>;
public:

    parser(const std::vector<core::token>& tokens, core::error_reporter& reporter);

    std::vector<stmt_ptr> parse();

private:
    const core::token& advance() noexcept;
    const core::token& peek() const noexcept;
    const core::token& prev() const noexcept;
    bool is_at_end() const noexcept;
    bool check(core::token_type type) const noexcept;
    bool match(std::initializer_list<core::token_type> types) noexcept;
    const core::token& consume(core::token_type type, std::string_view msg);

    stmt_ptr declaration();
    stmt_ptr statement();
    stmt_ptr var_declaration(core::value_type type, const core::token& name);
    stmt_ptr func_declaration(core::value_type return_type, const core::token& name);
    stmt_ptr while_statement();
    stmt_ptr if_statement();
    stmt_ptr block_statement();
    stmt_ptr return_statement();

    expr_ptr make_binary(expr_ptr left, core::token op, expr_ptr right);
    expr_ptr parse_binary(
        std::initializer_list<core::token_type> operators,
        std::function<expr_ptr()> sub_parser);
    expr_ptr expression();
    expr_ptr equality();
    expr_ptr assignment();
    expr_ptr comparison();
    expr_ptr term();
    expr_ptr factor();
    expr_ptr unary();
    expr_ptr primary();
    expr_ptr finish_call(const core::token& callee);

    ast::func_param parse_param();
    void synchronize();

    const std::vector<core::token>& tokens_;
    core::error_reporter& reporter_;
    size_t current_ = 0;
};

} 