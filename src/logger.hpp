#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <array>
#include <cassert>
#include <ctime>
#include <iostream>
#include <utility>

struct logger
{
    template <typename... Args>
    static void debug(Args... args)
    {
        if (instance().debug_enabled)
        {
            impl("DEBUG", std::forward<Args>(args)...);
        }
    }

    template <typename... Args>
    static void info(Args... args)
    {
        impl("INFO", std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void error(Args... args)
    {
        impl("ERROR", std::forward<Args>(args)...);
    }

    static void set_debug(const bool enabled)
    {
        instance().debug_enabled = enabled;
    }

private:
    static auto instance() -> logger&
    {
        static logger inst;
        return inst;
    }

    bool debug_enabled = false;

    template <typename... Args>
    static void impl(const char* lvl, Args... args)
    {
        constexpr auto max_date_str = 256;
        const auto now              = std::time(nullptr);
        const auto* local_now       = std::localtime(&now);
        auto buf                    = std::array<char, max_date_str>{};
        const auto written          = std::strftime(buf.data(), buf.size(), "%F %T", local_now);
        assert(written != 0);
        assert(written < 256);

        std::cerr << buf.data() << " " << lvl << " ";
        ((std::cerr << args << " "), ...);
        std::cerr << "\n";
    }
};

#endif    // LOGGER_HPP
