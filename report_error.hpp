#pragma once
#include <string_view>
#include <string>

void report_error(std::string_view text, size_t pos, std::string const& message);
