#include "core/token.hpp"
#include "core/error_report.hpp"
#include "core/type.hpp"
#include "lexer/lexer.hpp"
#include "parser/parser.hpp"
#include "ast/statement.hpp"
#include "ast/expression.hpp"
#include "semantics/symbol_table.hpp"
#include "semantics/type_check.hpp"
#include "runtime/value.hpp"
#include "runtime/environment.hpp"
#include "runtime/interpreter.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <vector>

int main() {
    std::string source = "int x = 10;\ndouble y = 3.14;\nint z = x + 5;\n";

    core::error_reporter reporter;
    lexer::lexer lex(source, reporter);
    auto tokens = lex.scan_tokens();
    if (reporter.has_error()) {
        std::cerr << "Lexical errors found.\n";
        return 1;
    }

    parser::parser p(tokens, reporter);
    auto ast = p.parse();
    if (reporter.has_error()) {
        std::cerr << "Syntax errors found.\n";
        return 1;
    }

    semantics::type_checker checker(reporter);
    bool sem_ok = checker.check(ast);
    if (!sem_ok) {
        std::cerr << "Semantic errors found.\n";
        return 1;
    }

    runtime::interpreter interpreter(reporter);
    interpreter.interpret(ast);
    if (reporter.has_error()) {
        std::cerr << "Runtime errors found.\n";
        return 1;
    }

    std::cout << "Program finished successfully.\n";
    return 0;
}