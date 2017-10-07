#include "test.hpp"

#include "exception.hpp"
#include "report_error.hpp"
#include "quote_esc.hpp"

#include "rpn.hpp"
#include "shunting_yard.hpp"
#include "recursive_descent.hpp"

#include <iostream>
#include <string>
using namespace std::string_literals;

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
