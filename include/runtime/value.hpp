// runtime/value.hpp

// This file defines the runtime value class, 
// which represents a value in the runtime environment. 
// It can hold different types of values (int, double, bool, string) 
// and provides methods for type conversion and basic operations.


#pragma once

#include "core/token.hpp"
#include <string>
#include <variant>
#include <optional>
#include <cstdint>


namespace runtime {

class value {
public:

    value();

	template <typename T>
	value(T v) : data_(std::move(v)) {}

    core::value_type type() const;

	int64_t to_int() const;
    double to_double() const;
    std::string to_string() const;

    template <typename T>
	std::optional<T> as() const noexcept {
		if (auto* p = std::get_if<T>(&data_)) return *p;
		return std::nullopt;
	}
    std::optional<int64_t> as_int() const noexcept;
    std::optional<double> as_double() const noexcept;
    std::optional<bool> as_bool() const noexcept;
    std::optional<std::string> as_string() const noexcept;

    value add(const value& other) const;
    value sub(const value& other) const;
    value mul(const value& other) const;
    value div(const value& other) const;
    value mod(const value& other) const;

    value eq(const value& other) const;
    value neq(const value& other) const;
    value lt(const value& other) const;
    value le(const value& other) const;
    value gt(const value& other) const;
    value ge(const value& other) const;

    value and_op(const value& other) const;
    value or_op(const value& other) const;
    value not_op() const;

private:
    std::variant<int64_t, double, bool, std::string, std::monostate> data_;
};

} // namespace runtime