// core/error_report.hpp

// The class provides methods to report errors, 
// check if any errors have been reported, and clear the error state. 
// The `error` method takes the line and column number where the error occurred, 
// along with an error message. The `has_error` method returns a boolean 
// indicating whether any errors have been reported, and the `clear_errors` 
// method resets the error state. The class uses a private method called `report`
// to handle the actual reporting of errors, and it maintains a boolean member variable 
// `had_error_` to track whether any errors have been reported.


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


} // namespace core