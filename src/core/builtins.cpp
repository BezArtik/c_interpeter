// core/builtins.cpp

// This file implements the built-in functions for the language. 
// These functions are available by default and can be called from user code. 
// Each built-in function is defined with its name, possible argument types (overloads),
// return type, and implementation.


#include "core/builtins.hpp"
#include "core/token_types.hpp"
#include "runtime/value.hpp"
#include <cmath>
#include <iostream>
#include <vector>
#include <sstream>

namespace core {

std::vector<builtin_def> builtins() {
    return {
        {
            "print",
            {{value_type::INT}, {value_type::DOUBLE}, {value_type::BOOL}, {value_type::STRING}, {}},
            value_type::VOID,
            [](const auto& args) {
                std::ostringstream oss;
                for (size_t i = 0; i < args.size(); ++i) {
                    if (i > 0) oss << " ";
                    oss << args[i].to_string();
                }
                std::cout << oss.str() << std::endl;
                return runtime::value();
            }
        },
        {
			"input",
			{{}},
			value_type::STRING,
			[](const auto& args) {
				std::string line;
				std::getline(std::cin, line);
				return runtime::value(line);
			}
        },
        {
            "sqrt",
            {{value_type::DOUBLE}, {value_type::INT}},
            value_type::DOUBLE,
            [](const auto& args) {
                double x = args[0].to_double();
                return runtime::value(std::sqrt(x));
            }
        },
        {
			"sin",
			{{value_type::DOUBLE}, {value_type::INT}},
			value_type::DOUBLE,
			[](const auto& args) {
				double x = args[0].to_double();
				return runtime::value(std::sin(x));
			}
	    },
        {
            "to_int",
            {{value_type::INT}, {value_type::DOUBLE}, {value_type::STRING}},
            value_type::INT,
            [](const auto& args) {
                const auto& a = args[0];
                if (a.type() == value_type::INT) return a;
                if (a.type() == value_type::DOUBLE) return runtime::value(static_cast<int64_t>(a.to_double()));
                return runtime::value(static_cast<int64_t>(std::stoll(a.to_string())));
            }
        },
        {
			"to_double",
			{{value_type::DOUBLE}, {value_type::INT}, {value_type::STRING}},
			value_type::DOUBLE,
			[](const auto& args) {
				const auto& a = args[0];
				if (a.type() == value_type::DOUBLE) return a;
				if (a.type() == value_type::INT) return runtime::value(static_cast<double>(a.to_int()));
				return runtime::value(std::stod(a.to_string()));
			}
        }

    };
}

} // namespace core