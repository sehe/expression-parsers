#pragma once
#include <ostream>
#include <iomanip>

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

