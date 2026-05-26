// type.cpp

// This file implements the type system for the core module. 
// It defines the value_type enum and provides 
// a function to convert it to a string representation.

#include "core/type.hpp"
#include <string>

namespace core {

std::string type_to_string(value_type type) noexcept {
	switch (type) {
	case value_type::INT:    return "int";
	case value_type::DOUBLE: return "double";
	case value_type::VOID:   return "void";
	case value_type::STRING: return "string";
	default:                 return "unknown";
	}
}

} // namespace core