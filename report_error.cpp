#include "report_error.hpp"
#include "quote_esc.hpp"
#include <algorithm>
#include <iostream>

void report_error(std::string_view text, size_t pos, std::string const& message) {
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


