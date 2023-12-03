#ifndef ERROR_HPP
#define ERROR_HPP

#include <format>
#include <expected>
#include <source_location>
#include <string>
#include <system_error>

struct error_t
{
    error_t(std::error_code ec, std::source_location caller_loc = std::source_location::current()): loc(caller_loc)
    {
        using namespace std::string_literals;
        message = std::format("{}:{}", ec.category().name(), ec.message());
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

auto operator<<(std::ostream& out, const error_t& err) -> std::ostream&;

template <typename T = void>
using result = std::expected<T, error_t>;

#endif    // ERROR_HPP
