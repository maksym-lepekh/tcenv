module;
#include <array>
#include <cassert>
#include <ctime>
#include <iostream>

export module log;

export struct log
{
    template <typename... Args>
    static void debug(Args... args)
    {
        impl("DEBUG", std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void info(Args... args)
    {
        if (instance().debug_enabled)
        {
            impl("INFO", std::forward<Args>(args)...);
        }
    }

    static void set_debug(bool enabled)
    {
        instance().debug_enabled = enabled;
    }

private:
    static auto instance() -> log&
    {
        static log inst;
        return inst;
    }

    bool debug_enabled = false;

    template <typename... Args>
    static void impl(const char* lvl, Args... args)
    {
        auto now       = std::time(nullptr);
        auto local_now = std::localtime(&now);
        auto buf       = std::array<char, 256>{};
        auto written   = std::strftime(buf.data(), buf.size(), "%F %T", local_now);
        assert(written != 0);
        assert(written < 256);

        std::cerr << buf.data() << " " << lvl << " ";
        ((std::cerr << args << " "), ...);
        std::cerr << "\n";
    }
};
