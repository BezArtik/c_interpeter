#include "core/type.hpp"
#include <string>

namespace core {

std::string type_to_string(value_type type_) {
	switch (type_) {
	case value_type::INT:    return "int";
	case value_type::DOUBLE: return "double";
	case value_type::VOID:   return "void";
	default:                 return "unknown";
	}
}

}