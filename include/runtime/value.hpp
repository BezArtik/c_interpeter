#pragma once

#include <string>
#include <variant>
#include <optional>
#include <cstdint>
#include "core/type.hpp"

namespace runtime {

class value {
public:

    value();

    value(int64_t v);
    value(double v);
    value(bool v);
    value(std::string v);

    core::value_type type() const;

    double to_double() const;
    std::string to_string() const;

    std::optional<int64_t> as_int() const;
    std::optional<double> as_double() const;
    std::optional<bool> as_bool() const;
    std::optional<std::string> as_string() const;

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