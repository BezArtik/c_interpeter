#pragma once

#include <string>

namespace core {

enum class value_type {
    INT,
    DOUBLE,
    BOOL,
    STRING,
    VOID,
    UNKNOWN
};

std::string type_to_string(value_type type);

} 