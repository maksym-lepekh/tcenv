#include <cstdlib>
#include <string_view>
import std;
import logger;
import recipe_repo;
import recipe_builder;

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
        auto b_env         = builder::get_env_for_pkg("sed");
        if (auto rec = repo.find_by_name("sed"))
        {
            logger::info("Found recipe for", pkg);
            builder::print_recipe(*rec);
            auto res = builder::build(*rec, b_env);
            if (!res)
            {
                logger::error(res.error());
                return EXIT_FAILURE;
            }
        }
    }

    return EXIT_SUCCESS;
}
