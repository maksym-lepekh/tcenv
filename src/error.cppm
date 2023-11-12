module;
#include <expected>
#include <ostream>
#include <source_location>
#include <string>
#include <system_error>

export module error;

using namespace std::literals;

export struct error_t
{
    error_t(std::error_code ec, std::source_location caller_loc = std::source_location::current()): loc(caller_loc)
    {
        message = ec.category().name() + ":"s + ec.message();
    }

    error_t(const std::exception& e, std::source_location caller_loc = std::source_location::current()):
        message(e.what()), loc(caller_loc)
    {}

    error_t(std::string msg, std::source_location caller_loc = std::source_location::current()):
        message(std::move(msg)), loc(caller_loc)
    {}

    std::string message;
    std::source_location loc;
};

export auto operator<<(std::ostream& out, error_t err) -> std::ostream&
{
    return out << err.message << "@" << err.loc.function_name() << "[" << err.loc.file_name() << ":" << err.loc.line()
               << ":" << err.loc.column();
}

export template <typename T>
using result = std::expected<T, error_t>;
