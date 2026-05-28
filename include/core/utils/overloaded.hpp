// core/overloaded.hpp

// This header defines a utility for creating 
// overloaded function objects from multiple lambda expressions 
// or function objects. It uses C++17's fold expressions to inherit 
// the operator() from all the provided types, allowing you to create
// a single callable object that can handle multiple types of arguments.

#pragma once

namespace core {

template<typename... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};

template<typename... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

} // namespace core