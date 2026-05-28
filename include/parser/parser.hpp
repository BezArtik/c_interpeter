// parser/parser.hpp

// The parser class is responsible for converting a
// sequence of tokens into an abstract syntax tree (AST).


#pragma once
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "core/token/token.hpp"
#include "core/error/error_report.hpp"
#include <vector>
#include <memory>
#include <string_view>
#include <functional>

namespace parser {

class parser {
public:

    parser(const std::vector<core::token>& tokens, core::error_reporter& reporter);

    std::vector<ast::stmt_ptr> parse();

private:
    const core::token& advance() noexcept;
    const core::token& peek() const noexcept;
    const core::token& prev() const noexcept;
    bool is_at_end() const noexcept;
    bool check(core::token_type type) const noexcept;
    bool match(std::initializer_list<core::token_type> types) noexcept;
    const core::token& consume(core::token_type type, core::error_code code);

    ast::stmt_ptr declaration();
    ast::stmt_ptr statement();
    ast::stmt_ptr var_declaration(core::value_type type, const core::token& name);
    ast::stmt_ptr func_declaration(core::value_type return_type, const core::token& name);
    ast::stmt_ptr while_statement();
	ast::stmt_ptr for_statement();
    ast::stmt_ptr if_statement();
    ast::stmt_ptr block_statement();
    ast::stmt_ptr return_statement();

    ast::expr_ptr make_binary(ast::expr_ptr left, core::token op, ast::expr_ptr right);
    ast::expr_ptr parse_binary(
        std::initializer_list<core::token_type> operators,
        std::function<ast::expr_ptr()> sub_parser);
    ast::expr_ptr expression();
    ast::expr_ptr equality();
    ast::expr_ptr assignment();
    ast::expr_ptr logic_or();
    ast::expr_ptr logic_and();
    ast::expr_ptr comparison();
    ast::expr_ptr term();
    ast::expr_ptr factor();
    ast::expr_ptr unary();

	ast::expr_ptr postfix_unary(ast::expr_ptr operand);
    ast::expr_ptr primary();
    ast::expr_ptr finish_call(const core::token& callee);

    ast::func_param parse_param();
    void synchronize();

	[[noreturn]] void error(const core::token& token, core::error_code code);

    const std::vector<core::token>& tokens_;
    core::error_reporter& reporter_;
    size_t current_ = 0;
};

} // namespace parser