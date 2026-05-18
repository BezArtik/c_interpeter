#pragma once

#include <string_view>

namespace core {

class error_reporter {
public:

	void error(size_t line, size_t column, std::string_view msg);

	bool has_error() const noexcept;
	void clear_errors() noexcept;

private:

	void report(size_t line, size_t column, std::string_view where, std::string_view msg);

	bool had_error_ = false;
};


}