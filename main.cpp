//#define BOOST_SPIRIT_X3_DEBUG
#include <iostream>
#include <iomanip>
#include <string_view>
#include "quote_esc.hpp"
#include "phoeni_x3.hpp"
namespace x3 = boost::spirit::x3;

namespace parsing {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wparentheses"

    using namespace PhoeniX3::placeholders;

    using Value = double;
    using x3::expect;
    x3::rule<struct _e, Value> expr   { "expr"   };
    x3::rule<struct _x, Value> expo   { "exponentation" };
    x3::rule<struct _t, Value> term   { "term"   };
    x3::rule<struct _f, Value> factor { "factor" };

    auto simple = x3::rule<struct _s, Value> {"simple"} 
        = x3::double_ [_val = _attr] 
        | '(' >> expect[term][_val = _attr] > ')'
        ;

    auto expo_def   = simple [_val = _attr] >> *(
                        '^' >> expect[expo] [ _val ^= _attr ]
                      )
                  ;

    auto factor_def = expo [_val = _attr] >> *(
                          '*' >> expect[factor] [_val *= _attr]
                        | '/' >> expect[factor] [_val /= _attr])
                  ;
    auto term_def = factor [_val = _attr] >> *(
                          '+' >> expect[term] [_val += _attr]
                        | '-' >> expect[term] [_val -= _attr])
                  ;

    auto expr_def = x3::skip(x3::space)[ x3::eps > term > x3::eoi ];

    BOOST_SPIRIT_DEFINE(expr, term, factor, expo)

    using expectation_failure = x3::expectation_failure<std::string_view::const_iterator>;

#pragma GCC diagnostic pop
}

int main() {
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
            "(2*2* 2* 2  * 2\t*2\n*2",
            })
    try {
        std::cout << "----------------------------------------------\n";
        double result;
        if (parse(text.begin(), text.end(), parsing::expr, result)) {
            std::cout << "OUTPUT: " << result << " ← " << quote_esc(text) << "\n";
        } else {
            std::cout << "Unparsed expression " << quote_esc(text) << "\n";
        }
    } catch(parsing::expectation_failure const& ef) {
        auto pos = ef.where() - text.begin();
        // isolate a line
        auto sol = text.find_last_of("\r\n", pos) + 1;
        auto eol = text.find_first_of("\r\n", pos);
        std::cout 
            << "Expecting " << ef.which() << " in " << quote_esc(text)
            << "\n  " << text.substr(sol, eol)
            << "\n  " << std::string(pos - sol, ' ') << "^--- here\n";
    }
}
