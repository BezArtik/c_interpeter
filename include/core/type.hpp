// This header defines an enumeration for 
// different value types and a function to convert 
// these types to their string representation. The `value_type` 
// enum class includes common data types such as 
// INT, DOUBLE, BOOL, STRING, VOID, and 
// an UNKNOWN type for unrecognized types. 
// The `type_to_string` function takes a `value_type` 
// and returns its corresponding string representation.


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

} // namespace core