#include "recipe_builder.hpp"
#include "recipe_repo.hpp"

#include <cstdlib>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <string_view>

using namespace std::literals;

auto main(int argc, char* argv[]) -> int
{
    if (argc != 2)
    {
        spdlog::info("wrong args");
        return EXIT_FAILURE;
    }

    spdlog::set_level(spdlog::level::debug);

    if (argv[1] == "test"sv)
    {
        spdlog::info("Selected 'test'");
        auto repo = recipe_repo{};
        repo.init();
        spdlog::info("Repo initialized");

        auto b_env = builder::get_env_for_pkg("sed");
        if (auto rec = repo.find_by_name("sed"))
        {
            constexpr auto pkg = "sed";
            spdlog::info("Found recipe for {}", pkg);
            builder::print_recipe(*rec);
            if (auto res = builder::build(*rec, b_env); !res)
            {
                spdlog::error(res.error());
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
