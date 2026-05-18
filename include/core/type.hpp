#pragma once

#include <string>
#include <cstdint>

namespace core {

enum class value_type : uint8_t {
    INT,
    DOUBLE,
    BOOL,
    STRING,
    VOID,
    UNKNOWN
};

std::string type_to_string(value_type type) noexcept;

} 