// parser/parser.cpp

// This file implements the parser for a programming language.

#include "parser/parser.hpp"
#include "core/token/token_types.hpp"
#include "core/token/keywords.hpp"
#include "core/error/error_codes.hpp"
#include <stdexcept>
#include <string>
#include <iostream>
#include <utility>
#include <algorithm>

namespace parser {

[[noreturn]] void parser::error(const core::token& token, core::error_code code) {
	reporter_.error(token.line_, token.column_, code);
	throw core::parse_error{ code, token.line_, token.column_ };
}

parser::parser(const std::vector<core::token>& tokens, core::error_reporter& reporter)
    : tokens_(tokens), reporter_(reporter) {
}

std::vector<ast::stmt_ptr> parser::parse() {
    std::vector<ast::stmt_ptr> statements_;
    while (!is_at_end()) {
        auto stmt = declaration();
        if (stmt) statements_.push_back(std::move(stmt));
    }
    return statements_;
}

bool parser::match(std::initializer_list<core::token_type> types) noexcept {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const core::token& parser::consume(core::token_type type, core::error_code code) {
    if (check(type)) return advance();
	error(peek(), code);
}

bool parser::check(core::token_type type) const noexcept {
    if (is_at_end()) return false;
    return peek().type_ == type;
}

const core::token& parser::advance() noexcept {
    if (!is_at_end()) current_++;
    return prev();
}

bool parser::is_at_end() const noexcept {
    return peek().type_ == core::token_type::END_OF_FILE;
}

const core::token& parser::peek() const noexcept {
    return tokens_[current_];
}

const core::token& parser::prev() const noexcept {
    return tokens_[current_ - 1];
}

ast::stmt_ptr parser::declaration() {
    try {
        for (auto tt : core::type_tokens) {
            if (match({ tt })) {
                auto type_opt = core::token_to_value_type(prev().type_);
                if (!type_opt) error(prev(), core::error_code::unknown_type);

                auto type = *type_opt;

                const auto& name = consume(core::token_type::IDENTIFIER, core::error_code::expected_identifier);

                if (match({ core::token_type::LEFT_PAREN })) {
                    return func_declaration(type, name);
                } else {
                    if (type == core::value_type::VOID) {
						error(prev(), core::error_code::void_variable);
                    }
                    return var_declaration(type, name);
                }
            }
        }
        return statement();
    } catch (const core::parse_error&) {
        synchronize();
        return nullptr;
    }
}

ast::stmt_ptr parser::var_declaration(core::value_type type, const core::token& name) {
    ast::expr_ptr initializer{};
    if (match({ core::token_type::EQUAL })) {
        initializer = expression();
    }

    consume(core::token_type::SEMICOLON, core::error_code::expected_semicolon);

    return std::make_unique<ast::statement>(
        ast::var_declaration(type, name, std::move(initializer))
    );
}

ast::stmt_ptr parser::func_declaration(core::value_type return_type, const core::token& name) {
    ast::func_declaration func(return_type, name);

    if (!check(core::token_type::RIGHT_PAREN)) {
        do {
            func.params_.push_back(parse_param());
        } while (match({ core::token_type::COMMA }));
    }

    consume(core::token_type::RIGHT_PAREN, core::error_code::expected_right_paren);
    consume(core::token_type::LEFT_BRACE, core::error_code::expected_left_brace);

    auto body = block_statement();

    auto& block = std::get<ast::block_stmt>(body->data_);
    func.body_ = std::make_unique<ast::block_stmt>(std::move(block));

    return std::make_unique<ast::statement>(std::move(func));
}

ast::func_param parser::parse_param() {
    core::value_type type{};
    bool found = false;

    for (auto tt : core::type_tokens) {
        if (match({ tt })) {
            auto type_opt = core::token_to_value_type(prev().type_);
            if (type_opt && *type_opt != core::value_type::VOID) {
                type = *type_opt;
                found = true;
                break;
            }
        }
    }

    if (!found) error(peek(), core::error_code::expected_type);

    const auto& name = consume(core::token_type::IDENTIFIER, core::error_code::expected_identifier);
    return {type, name};
}

ast::stmt_ptr parser::statement() {
    if (match({ core::token_type::WHILE })) return while_statement();
    if (match({ core::token_type::FOR })) return for_statement();
    if (match({ core::token_type::IF })) return if_statement();
    if (match({ core::token_type::RETURN })) return return_statement();
    if (match({ core::token_type::LEFT_BRACE })) return block_statement();

    auto expr_ = expression();
    consume(core::token_type::SEMICOLON, core::error_code::expected_semicolon);

    return std::make_unique<ast::statement>(
        ast::expression_stmt(std::move(expr_), prev().line_, prev().column_)
    );
}

ast::stmt_ptr parser::while_statement() {
    consume(core::token_type::LEFT_PAREN, core::error_code::expected_left_paren_while);
    auto condition_ = expression();
    consume(core::token_type::RIGHT_PAREN, core::error_code::expected_right_paren_condition);

    auto body_ = statement();
    return std::make_unique<ast::statement>(
        ast::while_stmt(std::move(condition_), std::move(body_), prev().line_, prev().column_)
    );
}

ast::stmt_ptr parser::for_statement() {
    consume(core::token_type::LEFT_PAREN, core::error_code::expected_left_paren_for);

    ast::stmt_ptr initializer{};
    if (match({ core::token_type::SEMICOLON })) {}
    else if (match({ core::token_type::INT_KEYWORD, core::token_type::DOUBLE_KEYWORD,
                     core::token_type::BOOL_KEYWORD, core::token_type::STRING_KEYWORD })) {
        auto type_opt = core::token_to_value_type(prev().type_);
        if (!type_opt) error(prev(), core::error_code::unknown_type);
        initializer = var_declaration(*type_opt,
            consume(core::token_type::IDENTIFIER, core::error_code::expected_identifier));
    } else {
        initializer = statement();
    }

    ast::expr_ptr condition{};
    if (!check(core::token_type::SEMICOLON)) condition = expression();
        
    consume(core::token_type::SEMICOLON, core::error_code::expected_semicolon);

    ast::expr_ptr increment{};
    if (!check(core::token_type::RIGHT_PAREN)) increment = expression();  

    consume(core::token_type::RIGHT_PAREN, core::error_code::expected_right_paren);

    auto body = statement();
    return std::make_unique<ast::statement>(
        ast::for_stmt(std::move(initializer), std::move(condition), 
                      std::move(increment), std::move(body),
                      prev().line_, prev().column_)
    );
}

ast::stmt_ptr parser::if_statement() {
    consume(core::token_type::LEFT_PAREN, core::error_code::expected_left_paren_if);
    auto condition = expression();
    consume(core::token_type::RIGHT_PAREN, core::error_code::expected_right_paren_condition);

    auto then_branch = statement();
    ast::stmt_ptr else_branch{};

    if (match({ core::token_type::ELSE })) else_branch = statement();
        
    return std::make_unique<ast::statement>(
        ast::if_stmt(std::move(condition), std::move(then_branch), 
                     std::move(else_branch), prev().line_, prev().column_)
    );
}

ast::stmt_ptr parser::return_statement() {
    const auto& keyword = prev();

    ast::expr_ptr value{};
    if (!check(core::token_type::SEMICOLON)) value = expression();

    consume(core::token_type::SEMICOLON, core::error_code::expected_semicolon);

    return std::make_unique<ast::statement>(
        ast::return_stmt(keyword, std::move(value))
    );
}

ast::stmt_ptr parser::block_statement() {
    ast::block_stmt block;
    while (!check(core::token_type::RIGHT_BRACE) && !is_at_end()) {
        auto stmt = declaration();
        if (stmt) block.statements_.push_back(std::move(stmt));
    }
    consume(core::token_type::RIGHT_BRACE, core::error_code::expected_right_brace);
    return std::make_unique<ast::statement>(std::move(block));
}

ast::expr_ptr parser::make_binary(ast::expr_ptr left, core::token op, ast::expr_ptr right) {
    return std::make_unique<ast::expression>(
        ast::binary_expr(std::move(left), op, std::move(right), op.line_, op.column_)
    );
}

ast::expr_ptr parser::parse_binary(
    std::initializer_list<core::token_type> operators,
    std::function<ast::expr_ptr()> sub_parser) {
    auto left = sub_parser();

    while (match(operators)) {
        const auto& op = prev();
        auto right = sub_parser();
        left = make_binary(std::move(left), op, std::move(right));
    }

    return left;
}

ast::expr_ptr parser::expression() {
    return assignment();
}

ast::expr_ptr parser::equality() {
    return parse_binary({ core::token_type::EQUAL_EQUAL, core::token_type::BANG_EQUAL },
        [this] { return comparison(); });
}

ast::expr_ptr parser::comparison() {
    return parse_binary({ core::token_type::GREATER, core::token_type::GREATER_EQUAL,
                          core::token_type::LESS, core::token_type::LESS_EQUAL },
        [this] { return term(); });
}

ast::expr_ptr parser::term() {
    return parse_binary({ core::token_type::PLUS, core::token_type::MINUS },
        [this] { return factor(); });
}

ast::expr_ptr parser::factor() {
    return parse_binary({ core::token_type::STAR, core::token_type::SLASH,
                          core::token_type::PERCENT },
        [this] { return unary(); });
}

ast::expr_ptr parser::logic_or() {
    return parse_binary({ core::token_type::OR },
        [this] { return logic_and(); });
}

ast::expr_ptr parser::logic_and() {
    return parse_binary({ core::token_type::AND },
        [this] { return equality(); });
}

ast::expr_ptr parser::assignment() {
    auto expr = logic_or();

    if (match({ core::token_type::EQUAL,
                core::token_type::PLUS_EQUAL,
                core::token_type::MINUS_EQUAL,
                core::token_type::STAR_EQUAL,
                core::token_type::SLASH_EQUAL,
                core::token_type::PERCENT_EQUAL })) {
        const auto& op = prev();
        auto value = assignment();
        return make_binary(std::move(expr), op, std::move(value));
    }
    return expr;
}

ast::expr_ptr parser::unary() {
    if (match({ core::token_type::BANG, core::token_type::MINUS,
                core::token_type::INCREMENT, core::token_type::DECREMENT })) {
        const auto& op = prev();
        auto operand = unary();
        return std::make_unique<ast::expression>(
            ast::unary_expr(op, std::move(operand), op.line_, op.column_)
        );
    }

    return primary();
}

ast::expr_ptr parser::postfix_unary(ast::expr_ptr operand) {
    const auto& op = prev();
    return std::make_unique<ast::expression>(
        ast::postfix_expr(std::move(operand), op, op.line_, op.column_)
    );
}

ast::expr_ptr parser::primary() {
    if (match({ core::token_type::NUMBER, core::token_type::STRING,
                core::token_type::TRUE, core::token_type::FALSE })) {
        return std::make_unique<ast::expression>(
            ast::literal_expr(prev(), prev().line_, prev().column_)
        );
    }

    if (match({ core::token_type::IDENTIFIER })) {
        const auto& name = prev();

        if (match({ core::token_type::LEFT_PAREN })) return finish_call(name);

        auto expr = std::make_unique<ast::expression>(
            ast::variable_expr(name, name.line_, name.column_)
        );

        if (match({ core::token_type::INCREMENT, core::token_type::DECREMENT })) {
            const auto& op = prev();
            return postfix_unary(std::move(expr));
        }

        return expr;
    }

    if (match({ core::token_type::LEFT_PAREN })) {
        auto expr = expression();
        consume(core::token_type::RIGHT_PAREN, core::error_code::expected_right_paren);
        return expr;
    }

	error(peek(), core::error_code::expected_expression);
}

ast::expr_ptr parser::finish_call(const core::token& callee) {
    ast::call_expr call(callee, std::vector<ast::expr_ptr>(), callee.line_, callee.column_);

    if (!check(core::token_type::RIGHT_PAREN)) {
        do {
            call.args_.push_back(expression());
        } while (match({ core::token_type::COMMA }));
    }

    consume(core::token_type::RIGHT_PAREN, core::error_code::expected_right_paren);

    return std::make_unique<ast::expression>(std::move(call));
}

void parser::synchronize() {
    advance();

    while (!is_at_end()) {
        if (prev().type_ == core::token_type::SEMICOLON) return;

        if (core::can_start_statement(peek().type_)) return;

        advance();
    }
}

} // namespace parser