// runtime/value.cpp

// This file implements the value class which represents a runtime value in the interpreter. 
// It supports various types and operations on them.


#include "runtime/value.hpp"
#include "core/token_types.hpp"
#include <stdexcept>
#include <cstdint>
#include <charconv>

namespace runtime {

value::value() : data_(std::monostate{}) {}

core::value_type value::type() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t>) return core::value_type::INT;
        else if constexpr (std::is_same_v<T, double>) return core::value_type::DOUBLE;
        else if constexpr (std::is_same_v<T, bool>) return core::value_type::BOOL;
        else if constexpr (std::is_same_v<T, std::string>) return core::value_type::STRING;
        else return core::value_type::VOID;
        }, data_);
}

int64_t value::to_int() const {
	if (auto i = as_int()) return static_cast<int64_t>(*i);
	if (auto d = as_double()) return static_cast<int64_t>(*d);
	if (auto s = as_string()) {
		int64_t result;
		auto [ptr, ec] = std::from_chars(s->data(), s->data() + s->size(), result);
		if (ec != std::errc{}) throw std::runtime_error("Cannot convert to int");
		return result;
	}
	throw std::runtime_error("Cannot convert to int");
}

double value::to_double() const {
    if (auto i = as_int()) return static_cast<double>(*i);
    if (auto d = as_double()) return *d;
    if (auto s = as_string()) {
		double result;
		auto [ptr, ec] = std::from_chars(s->data(), s->data() + s->size(), result);
		if (ec != std::errc{}) throw std::runtime_error("Cannot convert to double");
		return result;
    }
    throw std::runtime_error("Cannot convert to double");
}

std::string value::to_string() const {
    return std::visit([](auto&& arg) -> std::string {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int64_t>) return std::to_string(arg);
        else if constexpr (std::is_same_v<T, double>) return std::to_string(arg);
        else if constexpr (std::is_same_v<T, bool>) return arg ? "true" : "false";
        else if constexpr (std::is_same_v<T, std::string>) return arg;
        else return "void";
        }, data_);
}

std::optional<int64_t> value::as_int() const noexcept { return as<int64_t>(); }
std::optional<double> value::as_double() const noexcept { return as<double>(); }
std::optional<bool> value::as_bool() const noexcept { return as<bool>();}
std::optional<std::string> value::as_string() const noexcept { return as<std::string>();}

value value::add(const value& other) const {
    if (type() == core::value_type::INT && other.type() == core::value_type::INT) {
        return value(*as_int() + *other.as_int());
    }
    return value(to_double() + other.to_double());
}

value value::sub(const value& other) const {
    if (type() == core::value_type::INT && other.type() == core::value_type::INT) {
        return value(*as_int() - *other.as_int());
    }
    return value(to_double() - other.to_double());
}

value value::mul(const value& other) const {
    if (type() == core::value_type::INT && other.type() == core::value_type::INT) {
        return value(*as_int() * *other.as_int());
    }
    return value(to_double() * other.to_double());
}

value value::div(const value& other) const {
	if (type() == core::value_type::INT && other.type() == core::value_type::INT) {
		if (*other.as_int() == 0) throw std::runtime_error("Division by zero");
		return value(*as_int() / *other.as_int());
	}
    return value(to_double() / other.to_double());
}

value value::mod(const value& other) const {
    if (type() == core::value_type::INT && other.type() == core::value_type::INT) {
        if (*other.as_int() == 0) throw std::runtime_error("Modulo by zero");
        return value(*as_int() % *other.as_int());
    }
    throw std::runtime_error("Modulo supported only for integers");
}

value value::eq(const value& other) const {
    if (type() != other.type()) {
        if ((type() == core::value_type::INT || type() == core::value_type::DOUBLE) &&
            (other.type() == core::value_type::INT || other.type() == core::value_type::DOUBLE)) {
            return value(to_double() == other.to_double());
        }
        return value(false);
    }
    if (auto i = as_int()) return value(*i == *other.as_int());
    if (auto d = as_double()) return value(*d == *other.as_double());
    if (auto b = as_bool()) return value(*b == *other.as_bool());
    if (auto s = as_string()) return value(*s == *other.as_string());
    return value(false);
}

value value::neq(const value& other) const {
    return eq(other).not_op();
}

value value::lt(const value& other) const {
    if (type() != other.type()) throw std::runtime_error("Type mismatch in comparison");
    if (auto i = as_int()) return value(*i < *other.as_int());
    if (auto d = as_double()) return value(*d < *other.as_double());
    throw std::runtime_error("Comparison supported only for numeric types");
}

value value::le(const value& other) const {
    return lt(other).or_op(eq(other));
}

value value::gt(const value& other) const {
    return le(other).not_op();
}

value value::ge(const value& other) const {
    return lt(other).not_op();
}

value value::and_op(const value& other) const {
    if (auto b1 = as_bool()) {
        if (auto b2 = other.as_bool()) return value(*b1 && *b2);
    }
    throw std::runtime_error("Logical AND requires boolean operands");
}

value value::or_op(const value& other) const {
    if (auto b1 = as_bool()) {
        if (auto b2 = other.as_bool()) return value(*b1 || *b2);
    }
    throw std::runtime_error("Logical OR requires boolean operands");
}

value value::not_op() const {
    if (auto b = as_bool()) return value(!*b);
    throw std::runtime_error("Logical NOT requires boolean operand");
}


} // namespace runtime