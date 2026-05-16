#include "core/error_report.hpp"
#include <format>
#include <string_view>
#include <iostream>

namespace core {

void error_reporter::error(size_t line, size_t column, std::string_view msg) {
	report(line, column, "", msg);
}

bool error_reporter::has_error() const {
    return had_error_;
}

void error_reporter::clear_errors() {
    had_error_ = false;
}

void error_reporter::report(size_t line, size_t column, std::string_view where, std::string_view msg) {
    std::cerr << std::format("[line {}:{}] Error{}: {}\n", line, column, where, msg);
    had_error_ = true;
}

}