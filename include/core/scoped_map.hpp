// core/scoped_map.hpp

// This header defines a template class `scoped_map` 
// that manages a stack of scopes, each containing a 
// mapping of string keys to values of type T.


#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include <optional>
#include <functional>
#include <stdexcept>

namespace core {

template<typename T>
class scoped_map {
public:
    scoped_map() {
        scopes_.push_back(std::make_unique<scope>());
    }

    void push() {
        scopes_.push_back(std::make_unique<scope>());
    }

    void pop() {
        if (scopes_.size() > 1) scopes_.pop_back();
    }

    void define(const std::string& name, T value) {
        scopes_.back()->bindings_[name] = std::move(value);
    }

    std::optional<T> get(const std::string& name) const {
		auto it = std::find_if(scopes_.rbegin(), scopes_.rend(),
			[&](const auto& s) { return s->bindings_.contains(name); });
		if (it != scopes_.rend()) return (*it)->bindings_[name];
        return std::nullopt;
    }

    bool contains_in_current_scope(const std::string& name) const {
        return scopes_.back()->bindings_.contains(name);
    }

    void assign(const std::string& name, T value) {
        auto it = std::find_if(scopes_.rbegin(), scopes_.rend(),
            [&](const auto& s) { return s->bindings_.contains(name); });
		if (it != scopes_.rend()) {
			(*it)->bindings_[name] = std::move(value);
			return;
		}
        throw std::runtime_error("Assignment to undefined '" + name + "'");
    }

    void update_if_exists(const std::string& name, std::function<void(T&)> updater) {
		auto it = std::find_if(scopes_.rbegin(), scopes_.rend(), 
            [&](const auto& s) { return s->bindings_.contains(name); });
        if (it != scopes_.rend()) updater((*it)->bindings_[name]);
    }

private:

    struct scope {
        std::unordered_map<std::string, T> bindings_;
    };
    std::vector<std::unique_ptr<scope>> scopes_;

};

}