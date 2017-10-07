#pragma once
#include "../lexer.hpp"

namespace operators {
    using id = lexer::token_t::type_t;
    Value eval(id op, Value a, Value b);
}
