#pragma once
#include "exception.hpp"
#include "lexer.hpp"
#include "operators/logic.hpp"
#include <stack>
#include <vector>

namespace rpn {
    template <typename TokenIt>
    Value eval(TokenIt f, TokenIt l) {
        std::stack<Value> stack;

        auto pop = [&stack](std::optional<TokenIt> it = std::nullopt) {
            if (stack.empty()) {
                if (it) throw parse_failure(*it, "stack underrun at token '" + std::string((*it)->loc) + "'");
                else    throw std::runtime_error("void");
            }
            auto v = stack.top();
            stack.pop();
            return v;
        };

        for (; f != l; ++f) {
            switch (auto t = f->type) {
                case lexer::token_t::literal:
                    stack.push(*f->value);
                    break;
                case lexer::token_t::plus:
                case lexer::token_t::minus:
                case lexer::token_t::multiply:
                case lexer::token_t::divide:
                case lexer::token_t::exponentiate: {
                        auto b = pop(f), a = pop(f);
                        stack.push(operators::eval(t, a, b));
                    }
                    break;
                default:
                    throw parse_failure(f, "invalid token '" + std::string(f->loc) + "'");
            }
        }

        if (stack.size() > 1)
            throw std::runtime_error("unbalanced operand stack");
        return pop();
    }

    Value eval(std::string_view const& text) {
        std::vector<lexer::token_t> toks;

        tokenize(text.cbegin(), text.cend(), back_inserter(toks));
        return eval(toks.cbegin(), toks.cend());
    }
}


