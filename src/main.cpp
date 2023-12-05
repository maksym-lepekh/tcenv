#include "recipe_builder.hpp"
#include "recipe_repo.hpp"

#include <argparse/argparse.hpp>
#include <cstdlib>
#include <spdlog/fmt/ranges.h>
#include <spdlog/spdlog.h>
#include <string_view>

using namespace std::literals;

struct test_args: argparse::Args
{
    std::string& pkg_name = arg("Package to build").set_default("sed");

    int run() override
    {
        auto repo = recipe_repo{};
        repo.init();
        spdlog::info("Repo initialized");
        repo.print_list();

        if (auto rec = repo.find_by_name(pkg_name))
        {
            spdlog::info("Found recipe for {}", rec->package_name);
            builder::print_recipe(*rec);

            auto b_env = builder::get_env_for_pkg(rec.value());
            if (auto res = builder::build(*rec, b_env); !res)
            {
                spdlog::error(res.error());
                return EXIT_FAILURE;
            }
        }
        return EXIT_SUCCESS;
    }
};

struct app_args: argparse::Args
{
    test_args& cmd = subcommand("test");
    bool& verbose  = flag("v,verbose", "Enable more verbose output");
};

auto main(int argc, char* argv[]) -> int
{
    auto args = argparse::parse<app_args>(argc, argv);

    if (args.verbose)
    {
        spdlog::set_level(spdlog::level::debug);
    }

    return args.run_subcommands();
}
