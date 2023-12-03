#include "error.hpp"

#include <format>
#include <iostream>

auto operator<<(std::ostream& out, const error_t& err) -> std::ostream&
{
    return out << std::format("{}@{}[{}:{}:{}]", err.message, err.loc.function_name(), err.loc.file_name(),
                              err.loc.line(), err.loc.column());
}
