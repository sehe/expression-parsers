#pragma once
#include "exception.hpp"
#include "lexer.hpp"
#include "operators/logic.hpp"

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
                switch (auto t = f->type) {
                    case token_t::plus:
                    case token_t::minus:
                        operators::eval(t, accum, expect(term, ++f, l, "term"));
                        break;
                    default: return accum;
                }
            }

            return accum;
        }

        static Value factor(TokIt& f, TokIt l) {
            Value accum = exponentiation(f, l);

            while (f!=l) {
                switch (auto t = f->type) {
                    case token_t::multiply:
                    case token_t::divide:
                        operators::eval(t, accum, expect(factor, ++f, l, "factor"));
                        break;
                    default: return accum;
                }
            }

            return accum;
        }

        static Value exponentiation(TokIt& f, TokIt l) {
            Value accum = simple(f, l);

            while (f!=l) {
                switch (auto t = f->type) {
                    case token_t::exponentiate:
                        accum = operators::eval(t, accum, expect(exponentiation, ++f, l, "exponentiation"));
                        break;
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


