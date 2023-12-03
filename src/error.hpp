#ifndef ERROR_HPP
#define ERROR_HPP

#include <expected>
#include <format>
#include <source_location>
#include <spdlog/fmt/fmt.h>
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

template <typename T = void>
using result = std::expected<T, error_t>;

template <>
struct fmt::formatter<error_t>: fmt::formatter<std::string>
{
    auto format(error_t err, format_context& ctx) const -> decltype(ctx.out())
    {
        return fmt::format_to(ctx.out(), "{}@{}[{}:{}:{}]", err.message, err.loc.function_name(), err.loc.file_name(),
                              err.loc.line(), err.loc.column());
    }
};

#endif    // ERROR_HPP
