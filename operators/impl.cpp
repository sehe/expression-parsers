#include "logic.hpp"
#include "precedence.hpp"

#include <map>
#include <functional>
#include <cmath>

namespace operators {

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
