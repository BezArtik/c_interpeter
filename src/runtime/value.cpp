#include "runtime/value.hpp"
#include <stdexcept>

namespace runtime {

value::value() : data_(std::monostate{}) {}

value::value(int v) : data_(v) {}
value::value(double v) : data_(v) {}
value::value(bool v) : data_(v) {}
value::value(std::string v) : data_(std::move(v)) {}

core::value_type value::type() const {
    return std::visit([](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, int>) return core::value_type::INT;
        else if constexpr (std::is_same_v<T, double>) return core::value_type::DOUBLE;
        else if constexpr (std::is_same_v<T, bool>) return core::value_type::BOOL;
        else if constexpr (std::is_same_v<T, std::string>) return core::value_type::STRING;
        else return core::value_type::VOID;
        }, data_);
}

std::optional<int> value::as_int() const {
    if (auto* p = std::get_if<int>(&data_)) return *p;
    return std::nullopt;
}

std::optional<double> value::as_double() const {
    if (auto* p = std::get_if<double>(&data_)) return *p;
    return std::nullopt;
}

std::optional<bool> value::as_bool() const {
    if (auto* p = std::get_if<bool>(&data_)) return *p;
    return std::nullopt;
}

std::optional<std::string> value::as_string() const {
    if (auto* p = std::get_if<std::string>(&data_)) return *p;
    return std::nullopt;
}

static double to_number(const value& v) {
    if (auto i = v.as_int()) return static_cast<double>(*i);
    if (auto d = v.as_double()) return *d;
    throw std::runtime_error("Operand is not a number");
}

value value::add(const value& other) const {
    if (type() != other.type()) throw std::runtime_error("Type mismatch in addition");
    if (auto i = as_int()) return value(*i + *other.as_int());
    if (auto d = as_double()) return value(*d + *other.as_double());
    if (auto s = as_string()) return value(*s + *other.as_string());
    throw std::runtime_error("Addition not supported for these types");
}

value value::sub(const value& other) const {
    if (type() != other.type()) throw std::runtime_error("Type mismatch in subtraction");
    if (auto i = as_int()) return value(*i - *other.as_int());
    if (auto d = as_double()) return value(*d - *other.as_double());
    throw std::runtime_error("Subtraction not supported for these types");
}

value value::mul(const value& other) const {
    if (type() != other.type()) throw std::runtime_error("Type mismatch in multiplication");
    if (auto i = as_int()) return value(*i * *other.as_int());
    if (auto d = as_double()) return value(*d * *other.as_double());
    throw std::runtime_error("Multiplication not supported for these types");
}

value value::div(const value& other) const {
    if (type() != other.type()) throw std::runtime_error("Type mismatch in division");
    if (auto i = as_int()) {
        if (*other.as_int() == 0) throw std::runtime_error("Division by zero");
        return value(*i / *other.as_int());
    }
    if (auto d = as_double()) {
        if (*other.as_double() == 0.0) throw std::runtime_error("Division by zero");
        return value(*d / *other.as_double());
    }
    throw std::runtime_error("Division not supported for these types");
}

value value::mod(const value& other) const {
    if (type() != other.type()) throw std::runtime_error("Type mismatch in modulo");
    if (auto i = as_int()) {
        if (*other.as_int() == 0) throw std::runtime_error("Modulo by zero");
        return value(*i % *other.as_int());
    }
    throw std::runtime_error("Modulo supported only for integers");
}

value value::eq(const value& other) const {
    if (type() != other.type()) return value(false);
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


}