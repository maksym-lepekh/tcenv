#include "clangd_fixer.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <utility>

import logger;
import recipe;
import recipe_repo;
FIX_CLANGD_MODULES

using namespace std::literals;

auto main(int argc, char* argv[]) -> int
{
    if (argc != 2)
    {
        logger::info("wrong args");
        return EXIT_FAILURE;
    }

    logger::set_debug(true);

    if (argv[1] == "test"sv)
    {
        logger::info("Selected 'test'");
        auto repo = recipe_repo{};
        repo.init();
        logger::info("Repo initialized");

        constexpr auto pkg = "sed";
        auto b_env         = recipe_builder::get_env_for_pkg("sed");
        if (auto r = repo.find_by_name("sed"))
        {
            logger::info("Found recipe for", pkg);
            recipe_builder::print_recipe(*r);
            auto res = recipe_builder::build(*r, b_env);
            logger::info("Build result is", res);
            if (!res)
            {
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
