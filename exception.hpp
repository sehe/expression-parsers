#pragma once
#include <stdexcept>
#include <string>

template <typename CharOrTokenIt>
struct parse_failure : std::runtime_error {
    parse_failure(CharOrTokenIt where, std::string const& msg) : std::runtime_error(msg), _where(where) {}
    CharOrTokenIt where() const { return _where; }
  private:
    CharOrTokenIt _where;
};


