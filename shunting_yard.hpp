#pragma once
#include "exception.hpp"
#include "lexer.hpp"
#include "operators/precedence.hpp"
#include <vector>
#include <stack>
#include <cassert>

namespace shunting_yard {
    template <typename TokenIt>
    auto process(TokenIt f, TokenIt l) {
        using lexer::token_t;
        using operators::get_precedence;

        std::vector<lexer::token_t> output;
        std::stack<lexer::token_t> stack;

        auto TOS = [&stack] { assert(!stack.empty()); return stack.top(); };
        auto pop = [&stack] { assert(!stack.empty()); auto v = stack.top(); stack.pop(); return v; };

        for (; f != l; ++f) {
            switch (auto t = f->type) {
                case token_t::literal:
                    output.push_back(*f);
                    break;
                case token_t::plus:
                case token_t::minus:
                case token_t::multiply:
                case token_t::divide:
                case token_t::exponentiate:
                    while (!stack.empty() && get_precedence(TOS().type) >= get_precedence(t))
                        output.push_back(pop());
                    stack.push(*f);
                    break;
                case token_t::popen:
                    stack.push(*f);
                    break;
                case token_t::pclose:
                    while (!stack.empty() && TOS().type != token_t::popen)
                        output.push_back(pop());
                    pop(); //ignore
                    break;
                default:
                    throw parse_failure(f, "invalid token");
            }
        }
        while (!stack.empty())
            output.push_back(pop());

        return output;
    }
}


