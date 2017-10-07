#pragma once
#include "exception.hpp"
#include <string_view>
#include <optional>

using Value = double;

namespace lexer {
    struct token_t {
        std::string_view loc;
        enum type_t { literal, plus, minus, multiply, divide, exponentiate, popen, pclose } type;
        std::optional<Value> value;

        token_t(std::string_view loc, type_t type) : loc(loc), type(type) {}
        token_t(std::string_view loc, Value value) : loc(loc), type(literal), value(value) {}
    };

    template <typename CharIt, typename Out>
    Out tokenize(CharIt const f, CharIt const l, Out out) {
        auto it = f;
        while (it!=l) {
            while (it!=l && std::isspace(*it)) ++it;

            *out++ = [&]() -> token_t {
                switch (*it) {
                    case '(': return {std::string_view{it++,1}, token_t::popen};
                    case ')': return {std::string_view{it++,1}, token_t::pclose};
                    case '+': return {std::string_view{it++,1}, token_t::plus};
                    case '-': return {std::string_view{it++,1}, token_t::minus};
                    case '*': return {std::string_view{it++,1}, token_t::multiply};
                    case '/': return {std::string_view{it++,1}, token_t::divide};
                    case '^': return {std::string_view{it++,1}, token_t::exponentiate};
                    default: {
                        char const *start = &*it;
                        char *stop = nullptr;
                        auto value = std::strtod(start, &stop);
                        if (stop > start) {
                            std::advance(it, stop - start);
                            return {std::string_view(start, stop - start), value};
                        }
                    }
                }
                throw parse_failure(it, "invalid token");
            }();
        }

        return out;
    }
}

