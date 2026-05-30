// src/main.cpp

// This is the main entry point of the interpreter. It reads a source 
// file, performs lexical analysis, parsing, 
// semantic analysis, and then executes the program. 
// It also handles errors at each stage and reports them to the user.


#include "core/token/token.hpp"
#include "core/error/error_report.hpp"
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
#include <filesystem>


int main(int argc, char* argv[]) {

    try {

        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <source_file>\n";
            return 1;
        }

        std::ifstream file(argv[1]);
        if (!file) {
            std::cerr << "Error: cannot open file '" << argv[1] << "'\n";
            return 1;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        core::error_reporter reporter(source);
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
        if (!checker.check(ast)) {
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

	} catch (const std::exception& e) {
		std::cerr << "Fatal Error: " << e.what() << "\n";
		return 1;
	}

    return 0;
}