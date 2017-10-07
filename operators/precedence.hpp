#pragma once
#include "../lexer.hpp"

namespace operators {
    using id         = lexer::token_t::type_t;
    using precedence = unsigned char;

    precedence get_precedence(id op); 
}
