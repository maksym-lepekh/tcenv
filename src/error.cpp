#include "error.hpp"
#include <iostream>


auto operator<<(std::ostream& out, const error_t& err) -> std::ostream&
{
    return out << err.message << "@" << err.loc.function_name() << "[" << err.loc.file_name() << ":" << err.loc.line()
               << ":" << err.loc.column();
}
