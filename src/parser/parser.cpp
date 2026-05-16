#include "parser/parser.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include <stdexcept>
#include <memory>
#include <string>

namespace parser {

parser::parser(const std::vector<core::token>& tokens, core::error_reporter& reporter)
    : tokens_(tokens), reporter_(reporter) {
}

std::vector<parser::ptr_statement> parser::parse() {
    std::vector<ptr_statement> statements;
    while (!is_at_end()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }
    return statements;
}

bool parser::match(std::initializer_list<core::token_type> types) {
    for (auto type : types) {
        if (check(type)) {
            advance();
            return true;
        }
    }
    return false;
}

const core::token& parser::consume(core::token_type type, std::string_view msg) {
    if (check(type)) return advance();

    reporter_.error(peek().line_, peek().column_, msg);
    throw std::runtime_error("Parser error");
}

bool parser::check(core::token_type type) const {
    if (is_at_end()) return false;
    return peek().type_ == type;
}

const core::token& parser::advance() {
    if (!is_at_end()) current_++;
    return prev();
}

bool parser::is_at_end() const {
    return peek().type_ == core::token_type::END_OF_FILE;
}

const core::token& parser::peek() const {
    return tokens_[current_];
}

const core::token& parser::prev() const {
    return tokens_[current_ - 1];
}

std::unique_ptr<ast::statement> parser::declaration() {
    try {
        if (match({ core::token_type::INT_KEYWORD })) {
            return var_declaration(core::value_type::INT);
        }
        if (match({ core::token_type::DOUBLE_KEYWORD })) {
            return var_declaration(core::value_type::DOUBLE);
        }
        if (match({ core::token_type::BOOL_KEYWORD })) {
            return var_declaration(core::value_type::BOOL);
        }
        return statement();
    } catch (const std::runtime_error&) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<ast::statement> parser::var_declaration(core::value_type type) {
    const core::token& name = consume(core::token_type::IDENTIFIER, "Expect variable name.");

    std::unique_ptr<ast::expression> initializer = nullptr;
    if (match({ core::token_type::EQUAL })) {
        initializer = expression();
    }

    consume(core::token_type::SEMICOLON, "Expect ';' after variable declaration.");

    return std::make_unique<ast::var_declaration>(
        type,
        name,
        std::move(initializer)
    );
}

std::unique_ptr<ast::statement> parser::statement() {
    if (match({ core::token_type::WHILE })) {
        return while_statement();
    }
    if (match({ core::token_type::LEFT_BRACE })) {
        return block_statement();
    }

    auto expr = expression();
    consume(core::token_type::SEMICOLON, "Expect ';' after expression.");
    return std::make_unique<ast::expression_stmt>(std::move(expr));
}

std::unique_ptr<ast::statement> parser::while_statement() {
    consume(core::token_type::LEFT_PAREN, "Expect '(' after 'while'.");
    auto condition = expression();
    consume(core::token_type::RIGHT_PAREN, "Expect ')' after condition.");

    auto body = statement();

    return std::make_unique<ast::while_stmt>(std::move(condition), std::move(body));
}

std::unique_ptr<ast::statement> parser::block_statement() {
    std::vector<std::unique_ptr<ast::statement>> statements;

    while (!check(core::token_type::RIGHT_BRACE) && !is_at_end()) {
        auto stmt = declaration();
        if (stmt) {
            statements.push_back(std::move(stmt));
        }
    }

    consume(core::token_type::RIGHT_BRACE, "Expect '}' after block.");
    return std::make_unique<ast::block_stmt>(std::move(statements));
}

std::unique_ptr<ast::expression> parser::equality() {
    auto expr = primary();

    while (match({ core::token_type::EQUAL_EQUAL, core::token_type::BANG_EQUAL })) {
        core::token op = prev();
        auto right = primary();
        expr = std::make_unique<ast::binary_expr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<ast::expression> parser::primary() {
    if (match({ core::token_type::NUMBER, core::token_type::STRING,
                core::token_type::TRUE, core::token_type::FALSE })) {
        return std::make_unique<ast::literal_expr>(prev());
    }

    if (match({ core::token_type::IDENTIFIER })) {
        return std::make_unique<ast::variable_expr>(prev());
    }

    if (match({ core::token_type::LEFT_PAREN })) {
        auto expr = expression();
        consume(core::token_type::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    reporter_.error(peek().line_, peek().column_, "Expect expression.");
    throw std::runtime_error("Parser error");
}

std::unique_ptr<ast::expression> parser::expression() {
    return term();
}

std::unique_ptr<ast::expression> parser::term() {
    auto expr = factor();

    while (match({ core::token_type::PLUS, core::token_type::MINUS })) {
        core::token op = prev();
        auto right = factor();
        expr = std::make_unique<ast::binary_expr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

std::unique_ptr<ast::expression> parser::factor() {
    auto expr = primary();

    while (match({ core::token_type::STAR, core::token_type::SLASH })) {
        core::token op = prev();
        auto right = primary();
        expr = std::make_unique<ast::binary_expr>(std::move(expr), op, std::move(right));
    }

    return expr;
}

void parser::synchronize() {
    advance();

    while (!is_at_end()) {
        if (prev().type_ == core::token_type::SEMICOLON) return;

        switch (peek().type_) {
        case core::token_type::IF:
        case core::token_type::WHILE:
        case core::token_type::RETURN:
        case core::token_type::INT_KEYWORD:
        case core::token_type::DOUBLE_KEYWORD:
        case core::token_type::BOOL_KEYWORD:
            return;
        default:
            break;
        }

        advance();
    }
}

}