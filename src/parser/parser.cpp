#include "parser/parser.hpp"
#include <stdexcept>
#include <string>

namespace parser {

parser::parser(const std::vector<core::token>& tokens, core::error_reporter& reporter)
    : tokens_(tokens), reporter_(reporter) {
}

std::vector<std::unique_ptr<ast::statement>> parser::parse() {
    std::vector<std::unique_ptr<ast::statement>> statements_;
    while (!is_at_end()) {
        auto stmt = declaration();
        if (stmt) {
            statements_.push_back(std::move(stmt));
        }
    }
    return statements_;
}

bool parser::match(std::initializer_list<core::token_type> types) {
    for (auto type_ : types) {
        if (check(type_)) {
            advance();
            return true;
        }
    }
    return false;
}

const core::token& parser::consume(core::token_type type_, std::string_view msg) {
    if (check(type_)) return advance();
    reporter_.error(peek().line_, peek().column_, msg);
    throw std::runtime_error("Parser error");
}

bool parser::check(core::token_type type_) const {
    if (is_at_end()) return false;
    return peek().type_ == type_;
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
    }
    catch (const std::runtime_error&) {
        synchronize();
        return nullptr;
    }
}

std::unique_ptr<ast::statement> parser::var_declaration(core::value_type type_) {
    const core::token& name_ = consume(core::token_type::IDENTIFIER, "Expect variable name.");

    std::unique_ptr<ast::expression> initializer_ = nullptr;
    if (match({ core::token_type::EQUAL })) {
        initializer_ = expression();
    }

    consume(core::token_type::SEMICOLON, "Expect ';' after variable declaration.");

    auto decl = std::make_unique<ast::var_declaration>();
    decl->type_ = type_;
    decl->name_ = name_;
    decl->initializer_ = std::move(initializer_);

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*decl);
    return stmt;
}

std::unique_ptr<ast::statement> parser::statement() {
    if (match({ core::token_type::WHILE })) {
        return while_statement();
    }
    if (match({ core::token_type::IF })) {
        return if_statement();
    }
    if (match({ core::token_type::LEFT_BRACE })) {
        return block_statement();
    }

    auto expr_ = expression();
    consume(core::token_type::SEMICOLON, "Expect ';' after expression.");

    auto expr_stmt = std::make_unique<ast::expression_stmt>();
    expr_stmt->expr_ = std::move(expr_);

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*expr_stmt);
    return stmt;
}

std::unique_ptr<ast::statement> parser::while_statement() {
    consume(core::token_type::LEFT_PAREN, "Expect '(' after 'while'.");
    auto condition_ = expression();
    consume(core::token_type::RIGHT_PAREN, "Expect ')' after condition.");

    auto body_ = statement();

    auto while_stmt = std::make_unique<ast::while_stmt>();
    while_stmt->condition_ = std::move(condition_);
    while_stmt->body_ = std::move(body_);

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*while_stmt);
    return stmt;
}

std::unique_ptr<ast::statement> parser::if_statement() {
    consume(core::token_type::LEFT_PAREN, "Expect '(' after 'if'.");
    auto condition_ = expression();
    consume(core::token_type::RIGHT_PAREN, "Expect ')' after condition.");

    auto then_branch_ = statement();
    std::unique_ptr<ast::statement> else_branch_ = nullptr;

    if (match({ core::token_type::ELSE })) {
        else_branch_ = statement();
    }

    auto if_stmt = std::make_unique<ast::if_stmt>();
    if_stmt->condition_ = std::move(condition_);
    if_stmt->then_branch_ = std::move(then_branch_);
    if_stmt->else_branch_ = std::move(else_branch_);

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*if_stmt);
    return stmt;
}

std::unique_ptr<ast::statement> parser::block_statement() {
    auto block = std::make_unique<ast::block_stmt>();

    while (!check(core::token_type::RIGHT_BRACE) && !is_at_end()) {
        auto stmt = declaration();
        if (stmt) {
            block->statements_.push_back(std::move(stmt));
        }
    }

    consume(core::token_type::RIGHT_BRACE, "Expect '}' after block.");

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*block);
    return stmt;
}

std::unique_ptr<ast::expression> parser::expression() {
    return equality();
}

std::unique_ptr<ast::expression> parser::equality() {
    auto expr_ = comparison();

    while (match({ core::token_type::EQUAL_EQUAL, core::token_type::BANG_EQUAL })) {
        core::token op = prev();
        auto right = comparison();

        auto binary = std::make_unique<ast::binary_expr>();
        binary->left_ = std::move(expr_);
        binary->op_ = op;
        binary->right_ = std::move(right);

        auto new_expr = std::make_unique<ast::expression>();
        new_expr->data_ = std::move(*binary);
        expr_ = std::move(new_expr);
    }

    return expr_;
}

std::unique_ptr<ast::expression> parser::comparison() {
    auto expr_ = term();

    while (match({ core::token_type::GREATER, core::token_type::GREATER_EQUAL,
                    core::token_type::LESS, core::token_type::LESS_EQUAL })) {
        core::token op = prev();
        auto right = term();

        auto binary = std::make_unique<ast::binary_expr>();
        binary->left_ = std::move(expr_);
        binary->op_ = op;
        binary->right_ = std::move(right);

        auto new_expr = std::make_unique<ast::expression>();
        new_expr->data_ = std::move(*binary);
        expr_ = std::move(new_expr);
    }

    return expr_;
}

std::unique_ptr<ast::expression> parser::term() {
    auto expr_ = factor();

    while (match({ core::token_type::PLUS, core::token_type::MINUS })) {
        core::token op = prev();
        auto right = factor();

        auto binary = std::make_unique<ast::binary_expr>();
        binary->left_ = std::move(expr_);
        binary->op_ = op;
        binary->right_ = std::move(right);

        auto new_expr = std::make_unique<ast::expression>();
        new_expr->data_ = std::move(*binary);
        expr_ = std::move(new_expr);
    }

    return expr_;
}

std::unique_ptr<ast::expression> parser::factor() {
    auto expr_ = unary();

    while (match({ core::token_type::STAR, core::token_type::SLASH, core::token_type::PERCENT })) {
        core::token op = prev();
        auto right = unary();

        auto binary = std::make_unique<ast::binary_expr>();
        binary->left_ = std::move(expr_);
        binary->op_ = op;
        binary->right_ = std::move(right);

        auto new_expr = std::make_unique<ast::expression>();
        new_expr->data_ = std::move(*binary);
        expr_ = std::move(new_expr);
    }

    return expr_;
}

std::unique_ptr<ast::expression> parser::unary() {
    if (match({ core::token_type::BANG, core::token_type::MINUS })) {
        core::token op = prev();
        auto operand = unary();

        auto unary_expr = std::make_unique<ast::unary_expr>();
        unary_expr->op_ = op;
        unary_expr->operand_ = std::move(operand);

        auto new_expr = std::make_unique<ast::expression>();
        new_expr->data_ = std::move(*unary_expr);
        return new_expr;
    }

    return primary();
}

std::unique_ptr<ast::expression> parser::primary() {
    if (match({ core::token_type::NUMBER, core::token_type::STRING,
                core::token_type::TRUE, core::token_type::FALSE })) {
        auto lit = std::make_unique<ast::literal_expr>();
        lit->value_ = prev();

        auto expr_ = std::make_unique<ast::expression>();
        expr_->data_ = std::move(*lit);
        return expr_;
    }

    if (match({ core::token_type::IDENTIFIER })) {
        auto var = std::make_unique<ast::variable_expr>();
        var->name_ = prev();

        auto expr_ = std::make_unique<ast::expression>();
        expr_->data_ = std::move(*var);
        return expr_;
    }

    if (match({ core::token_type::LEFT_PAREN })) {
        auto expr_ = expression();
        consume(core::token_type::RIGHT_PAREN, "Expect ')' after expression.");
        return expr_;
    }

    reporter_.error(peek().line_, peek().column_, "Expect expression.");
    throw std::runtime_error("Parser error");
}

void parser::synchronize() {
    advance();

    while (!is_at_end()) {
        if (prev().type_ == core::token_type::SEMICOLON) return;

        switch (peek().type_) {
        case core::token_type::IF:
        case core::token_type::ELSE:
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