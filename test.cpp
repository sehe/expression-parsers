#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <optional>
#include <string_view>
#include <vector>

using namespace std::string_literals;
using Value = double;

template <typename CharOrTokenIt>
struct parse_failure : std::runtime_error {
    parse_failure(CharOrTokenIt where, std::string const& msg) : std::runtime_error(msg), _where(where) {}
    CharOrTokenIt where() const { return _where; }
  private:
    CharOrTokenIt _where;
};

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

namespace recursive_descent {
    using namespace lexer;

    template <typename TokIt> struct parser {
        static Value expression(TokIt& f, TokIt l) {
            auto r = expect(term, f, l, "expression");
            return f==l? r : throw parse_failure(f, "Expected EOI");
        }

      private:
        using Parser = Value(TokIt&, TokIt);
        static Value expect(Parser* pf, TokIt& f, TokIt l, std::string const& what) {
            auto saved = f, result = pf(f, l);
            return f!=saved? result : throw parse_failure(f, "Expected " + what);
        };

        static Value term(TokIt& f, TokIt l) {
            Value accum = factor(f, l);

            while (f!=l) {
                switch (f->type) {
                    case token_t::plus:  accum += expect(term, ++f, l, "term"); break;
                    case token_t::minus: accum -= expect(term, ++f, l, "term"); break;
                    default: return accum;
                }
            }

            return accum;
        }

        static Value factor(TokIt& f, TokIt l) {
            Value accum = exponentiation(f, l);

            while (f!=l) {
                switch (f->type) {
                    case token_t::multiply: accum *= expect(factor, ++f, l, "factor"); break;
                    case token_t::divide:   accum /= expect(factor, ++f, l, "factor"); break;
                    default: return accum;
                }
            }

            return accum;
        }

        static Value exponentiation(TokIt& f, TokIt l) {
            Value accum = simple(f, l);

            while (f!=l) {
                switch (f->type) {
                    case token_t::exponentiate: accum = pow(accum, expect(exponentiation, ++f, l, "exponentiation")); break;
                    default: return accum;
                }
            }

            return accum;
        }

        static Value simple(TokIt& f, TokIt l) {

            if (f!=l) {
                switch (f->type) {
                    case token_t::literal: return *(f++)->value;
                    case token_t::popen:
                       {
                           auto v = expect(term, ++f, l, "sub expression");

                           if (f!=l && f->type == token_t::pclose)
                               ++f;
                           else
                               throw parse_failure(f, "Expect ')'");

                           return v;
                       }
                    default:
                       break;
                }
            }

            return std::numeric_limits<Value>::quiet_NaN(); // the guard will fire
        }
    };

    template <typename TokIt>
        Value parse(TokIt f, TokIt l) {
            return parser<TokIt>::expression(f, l);
        }
}

#include <map>
#include <functional>

namespace operators {
    using id         = lexer::token_t::type_t;
    using precedence = unsigned char;

    struct operator_def {
        precedence                        level;
        std::function<Value(Value,Value)> eval;
    };

    static std::map<id, operator_def> defs {
        { id::plus,         {1, std::plus<>{}} },
        { id::minus,        {1, std::minus<>{}} },
        { id::multiply,     {2, std::multiplies<>{}} },
        { id::divide,       {2, std::divides<>{}} },
        { id::exponentiate, {3, [](Value a, Value b) { return pow(a, b); }}  },
    };

    precedence get_precedence(id op) {
        auto it = defs.find(op);
        return it == defs.end()? 0 : defs.at(op).level;
    } 

    Value eval(id op, Value a, Value b) {
        return defs.at(op).eval(a, b);
    }
}

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

template <typename T>
struct quote_esc {
    T sv;
    quote_esc(T sv):sv(std::move(sv)) {}

    friend std::ostream& operator<<(std::ostream& os, quote_esc const& esc) {
        os << '"';
        for(uint8_t ch : std::string_view(esc.sv))
            switch(ch) {
                case '"' : os << "\\\""; break;
                case '\r': os << "\\r";  break;
                case '\n': os << "\\n";  break;
                case '\b': os << "\\b";  break;
                case '\0': os << "\\0";  break;
                case '\t': os << "\\t";  break;
                case '\f': os << "\\f";  break;
                default: os << ch;
            }

        return os << '"';
    }
};

static void report_error(std::string_view text, size_t pos, std::string const& message) {
    // isolate a line
    auto const sol = text.find_last_of("\r\n", pos) + 1;
    auto const eol = text.find_first_of("\r\n", pos);
    // calculate line:col
    auto const line = 1 + std::count(text.begin(), text.begin()+sol, '\n');
    auto const col = pos - sol;
    std::cout << "text|" << line << " col " << col << "| " << message << " in ";
    std::cout << quote_esc(text)
        << "\n  " << text.substr(sol, eol)
        << "\n  " << std::string(col, ' ') << "^--- here\n";
}

template <typename Parser>
void run_parse_tests(std::string const& caption, Parser parser) {
    for (std::string_view text : {
            "1 + ( 2 - 3 )", //OUTPUT: 0.0
            "( ( 1 + ( 2 - 3 ) + 5 ) / 2 ) ^ 2", //OUTPUT: 6.25
            "5 + ( ( 1 + 2 ) * 4 ) - 3", //OUTPUT: 14.0
            "3+4",
            "3+4*2",
            "3+(4*2)",
            "(3+4)*2",
            "2*2* 2* 2  * 2\t*2\n*2",
            "(2",
            "(2+/",
            "42*()",
            "42*(1) (8)",
            "(2*2* 2* 2  * 2\t*2\n*2",
            "(2*2* a* 2  * 2\t*2\n*2",
            "3^2^1",
            "4^3^2",
        })
    {
        std::vector<lexer::token_t> toks;

        try {
            std::cout << "----------------------------------------------\n";
            tokenize(text.cbegin(), text.cend(), back_inserter(toks));
            Value result = parser(toks.cbegin(), toks.cend());

            std::cout << caption << result << " ← " << quote_esc(text) << "\n";
        } catch(parse_failure<std::string_view::const_iterator> const& ef) {
            auto const pos = ef.where() - text.begin();
            report_error(text, pos, caption + " Tokenize error: "s + ef.what());
        } catch(parse_failure<std::vector<lexer::token_t>::const_iterator> const& ef) {
            auto pos = ef.where() == toks.end()? text.length() : ef.where()->loc.data() - text.data();
            report_error(text, pos, caption + " Parse error: "s + ef.what());
        } catch(std::exception const& e) {
            std::cout << caption << " " << e.what() << "\n";
        }
    }
}

void test_recursive_descent() {
    run_parse_tests("RecursiveDescent: ", [](auto f, auto l) { return recursive_descent::parse(f, l); });
}

void test_shunting_yard() {
    run_parse_tests("SY+RPN:", [](auto f, auto l) { 
        auto rpn = shunting_yard::process(f, l); 
#ifdef DEBUG_SHUNTINGYARD
        std::cout << "DEBUG: " << quote_esc(f->loc.data());
        std::cout << "\nDEBUG TOK: "; for (auto it = f; it != l; ++it) std::cout << '[' << it->loc << ']';
        std::cout << "\nDEBUG RPN: "; for (auto& tok : rpn) std::cout << '[' << tok.loc << ']';
        std::cout << "\n";
#endif

        return rpn::eval(rpn.cbegin(), rpn.cend());
    });
}

void test_rpn() {
    for (std::string_view text : {
            "1 2 +",       // OUTPUT: 3
            "3 2 - 1 +",   // OUTPUT: 2
            "3 ( 2 - 1 +", // error
            "42",          // ok
            "42 666",      // go away, satan
            "42 333 /",    // ok
            "42 666 666 -",
            "42 333 333 - +",    // ok
            "666 42 666 666 - +", 
            "8 /",
            ""
    }) try {
        std::cout << "RPN('" << text << "') → " << rpn::eval(text) << "\n";
    } catch(parse_failure<std::vector<lexer::token_t>::const_iterator> const& ef) {
        auto pos = ef.where()->loc.data() - text.data();
        report_error(text, pos, "RPN Eval error: "s + ef.what());
    } catch(std::exception const& e) {
        std::cout << "RPN: " << e.what() << "\n";
    }
}

int main() {
    test_rpn();
    test_recursive_descent();
    test_shunting_yard();
}
