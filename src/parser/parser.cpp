// parser/parser.cpp

// This file implements the parser for a programming language.

#include "parser/parser.hpp"
#include <stdexcept>
#include <string>
#include <iostream>
#include <utility>
#include <algorithm>

namespace parser {

parser::parser(const std::vector<core::token>& tokens, core::error_reporter& reporter)
    : tokens_(tokens), reporter_(reporter) {
}

std::vector<parser::stmt_ptr> parser::parse() {
    std::vector<stmt_ptr> statements_;
    while (!is_at_end()) {
        auto stmt = declaration();
        if (stmt) {
            statements_.push_back(std::move(stmt));
        }
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

const core::token& parser::consume(core::token_type type, std::string_view msg) {
    if (check(type)) return advance();
    reporter_.error(peek().line_, peek().column_, msg);
    throw std::runtime_error("Parser error");
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

parser::stmt_ptr parser::declaration() {
    try {
        for (auto tt : core::type_tokens) {
            if (match({ tt })) {
                auto type_opt = core::token_to_value_type(prev().type_);
                if (!type_opt) throw std::runtime_error("Unknown type");
                auto type = *type_opt;

                auto name = consume(core::token_type::IDENTIFIER, "Expect name after type.");

                if (match({ core::token_type::LEFT_PAREN })) {
                    return func_declaration(type, name);
                } else {
                    if (type == core::value_type::VOID) {
                        reporter_.error(name.line_, name.column_,
                            "cannot declare variable of type void");
                        throw std::runtime_error("Parser error");
                    }
                    return var_declaration(type, name);
                }
            }
        }
        return statement();
    } catch (const std::runtime_error&) {
        synchronize();
        return nullptr;
    }
}

parser::stmt_ptr parser::var_declaration(core::value_type type, const core::token& name) {
    parser::expr_ptr initializer = nullptr;
    if (match({ core::token_type::EQUAL })) {
        initializer = expression();
    }

    consume(core::token_type::SEMICOLON, "Expect ';' after variable declaration.");

    auto decl = std::make_unique<ast::var_declaration>();
    decl->type_ = type;
    decl->name_ = name;
    decl->initializer_ = std::move(initializer);

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*decl);
    return stmt;
}

parser::stmt_ptr parser::func_declaration(core::value_type return_type, const core::token& name) {
    auto func = std::make_unique<ast::func_declaration>();
    func->return_type_ = return_type;
    func->name_ = name;

    if (!check(core::token_type::RIGHT_PAREN)) {
        do {
            func->params_.push_back(parse_param());
        } while (match({ core::token_type::COMMA }));
    }

    consume(core::token_type::RIGHT_PAREN, "Expect ')' after parameters.");

    consume(core::token_type::LEFT_BRACE, "Expect '{' before function body.");
    auto body = block_statement();

    auto& block = std::get<ast::block_stmt>(body->data_);
    func->body_ = std::make_unique<ast::block_stmt>(std::move(block));

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*func);
    return stmt;
}

ast::func_param parser::parse_param() {
    core::value_type type;
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

    if (!found) {
        throw std::runtime_error("Expect parameter type.");
    }

    auto name = consume(core::token_type::IDENTIFIER, "Expect parameter name.");

	ast::func_param param{ type, name };
    return param;
}

parser::stmt_ptr parser::statement() {
    if (match({ core::token_type::WHILE })) return while_statement();
	if (match({ core::token_type::FOR })) return for_statement();
    if (match({ core::token_type::IF })) return if_statement();
    if (match({ core::token_type::RETURN })) return return_statement();
    if (match({ core::token_type::LEFT_BRACE })) return block_statement();
        
    auto expr_ = expression();
    consume(core::token_type::SEMICOLON, "Expect ';' after expression.");

    auto expr_stmt = std::make_unique<ast::expression_stmt>();
    expr_stmt->expr_ = std::move(expr_);

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*expr_stmt);
    return stmt;
}

parser::stmt_ptr parser::while_statement() {
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

parser::stmt_ptr parser::for_statement() {
	consume(core::token_type::LEFT_PAREN, "Expect '(' after 'for'.");
	parser::stmt_ptr initializer = nullptr;
	if (match({ core::token_type::SEMICOLON })) {}
	else if (match({ core::token_type::INT_KEYWORD, core::token_type::DOUBLE_KEYWORD,
					core::token_type::BOOL_KEYWORD, core::token_type::STRING_KEYWORD })) {
		auto type_opt = core::token_to_value_type(prev().type_);
		if (!type_opt) throw std::runtime_error("Unknown type");
		initializer = var_declaration(*type_opt, consume(core::token_type::IDENTIFIER, "Expect variable name."));
	} else {
		initializer = statement();
	}
	parser::expr_ptr condition = nullptr;
	if (!check(core::token_type::SEMICOLON)) {
		condition = expression();
	}
	consume(core::token_type::SEMICOLON, "Expect ';' after loop condition.");
	parser::expr_ptr increment = nullptr;
	if (!check(core::token_type::RIGHT_PAREN)) {
		increment = expression();
	}
	consume(core::token_type::RIGHT_PAREN, "Expect ')' after for clauses.");
	auto body = statement();
	auto for_stmt = std::make_unique<ast::for_stmt>();
	for_stmt->initializer_ = std::move(initializer);
	for_stmt->condition_ = std::move(condition);
	for_stmt->increment_ = std::move(increment);
	for_stmt->body_ = std::move(body);
	auto stmt = std::make_unique<ast::statement>();
	stmt->data_ = std::move(*for_stmt);
	return stmt;
}

parser::stmt_ptr parser::if_statement() {
    consume(core::token_type::LEFT_PAREN, "Expect '(' after 'if'.");
    auto condition_ = expression();
    consume(core::token_type::RIGHT_PAREN, "Expect ')' after condition.");

    auto then_branch_ = statement();
    parser::stmt_ptr else_branch_ = nullptr;

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

parser::stmt_ptr parser::return_statement() {
    auto keyword = prev();

    parser::expr_ptr value = nullptr;
    if (!check(core::token_type::SEMICOLON)) {
        value = expression();
    }

    consume(core::token_type::SEMICOLON, "Expect ';' after return value.");

    auto ret = std::make_unique<ast::return_stmt>();
    ret->keyword_ = keyword;
    ret->value_ = std::move(value);

    auto stmt = std::make_unique<ast::statement>();
    stmt->data_ = std::move(*ret);
    return stmt;
}

parser::stmt_ptr parser::block_statement() {
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

parser::expr_ptr parser::make_binary(expr_ptr left, core::token op, expr_ptr right) {
    auto binary = std::make_unique<ast::binary_expr>();
    binary->left_ = std::move(left);
    binary->op_ = op;
    binary->right_ = std::move(right);

    auto expr = std::make_unique<ast::expression>();
    expr->data_ = std::move(*binary);
    return expr;
}

parser::expr_ptr parser::parse_binary(
    std::initializer_list<core::token_type> operators,
    std::function<expr_ptr()> sub_parser) {
    auto left = sub_parser();

    while (match(operators)) {
        core::token op = prev();
        auto right = sub_parser();
        left = make_binary(std::move(left), op, std::move(right));
    }

    return left;
}

parser::expr_ptr parser::expression() {
    return assignment();
}

parser::expr_ptr parser::equality() {
    return parse_binary({ core::token_type::EQUAL_EQUAL, core::token_type::BANG_EQUAL },
        [this] { return comparison(); });
}

parser::expr_ptr parser::comparison() {
    return parse_binary({ core::token_type::GREATER, core::token_type::GREATER_EQUAL,
                          core::token_type::LESS, core::token_type::LESS_EQUAL },
        [this] { return term(); });
}

parser::expr_ptr parser::term() {
    return parse_binary({ core::token_type::PLUS, core::token_type::MINUS },
        [this] { return factor(); });
}

parser::expr_ptr parser::factor() {
    return parse_binary({ core::token_type::STAR, core::token_type::SLASH, 
                          core::token_type::PERCENT },
        [this] { return unary(); });
}

parser::expr_ptr parser::logic_or() {
    return parse_binary({ core::token_type::OR },
        [this] { return logic_and(); });
}

parser::expr_ptr parser::logic_and() {
    return parse_binary({ core::token_type::AND },
        [this] { return equality(); });
}

parser::expr_ptr parser::assignment() {
    auto expr = logic_or();

    if (match({ core::token_type::EQUAL,
               core::token_type::PLUS_EQUAL,
               core::token_type::MINUS_EQUAL,
               core::token_type::STAR_EQUAL,
               core::token_type::SLASH_EQUAL,
               core::token_type::PERCENT_EQUAL })) {
        core::token op = prev();
        auto value = assignment();
        return make_binary(std::move(expr), op, std::move(value));
    }
    return expr;
}

parser::expr_ptr parser::unary() {
    if (match({ core::token_type::BANG, core::token_type::MINUS,
                core::token_type::INCREMENT, core::token_type::DECREMENT })) {
        auto op = prev();
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

parser::expr_ptr parser::postfix_unary(parser::expr_ptr operand) {
    core::token op = prev();

    auto postfix = std::make_unique<ast::postfix_expr>();
    postfix->op_ = op;
    postfix->operand_ = std::move(operand);

    auto expr = std::make_unique<ast::expression>();
    expr->data_ = std::move(*postfix);
    return expr;
}

parser::expr_ptr parser::primary() {
    if (match({ core::token_type::NUMBER, core::token_type::STRING,
               core::token_type::TRUE, core::token_type::FALSE })) {
        auto lit = std::make_unique<ast::literal_expr>();
        lit->value_ = prev();

        auto expr = std::make_unique<ast::expression>();
        expr->data_ = std::move(*lit);
        return expr;
    }

    if (match({ core::token_type::IDENTIFIER })) {
        auto name = prev();

        if (match({ core::token_type::LEFT_PAREN })) {
            return finish_call(name);
        }

        auto var = std::make_unique<ast::variable_expr>();
        var->name_ = name;

        auto expr = std::make_unique<ast::expression>();
        expr->data_ = std::move(*var);

		if (match({ core::token_type::INCREMENT, core::token_type::DECREMENT })) {
			auto op = prev();
			return postfix_unary(std::move(expr));
		}

        return expr;
    }

    if (match({ core::token_type::LEFT_PAREN })) {
        auto expr = expression();
        consume(core::token_type::RIGHT_PAREN, "Expect ')' after expression.");
        return expr;
    }

    reporter_.error(peek().line_, peek().column_, "Expect expression.");
    throw std::runtime_error("Parser error");
}

parser::expr_ptr parser::finish_call(const core::token& callee) {
    auto call = std::make_unique<ast::call_expr>();
    call->callee_ = callee;

    if (!check(core::token_type::RIGHT_PAREN)) {
        do {
            call->args_.push_back(expression());
        } while (match({ core::token_type::COMMA }));
    }

    consume(core::token_type::RIGHT_PAREN, "Expect ')' after arguments.");

    auto expr = std::make_unique<ast::expression>();
    expr->data_ = std::move(*call);
    return expr;
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