#include "recipe_builder.hpp"

#include "control_flow.hpp"
#include "download_step.hpp"
#include "logger.hpp"

#include <boost/process.hpp>
#include <toml++/toml.hpp>

namespace builder
{
    auto from_toml(std::string_view input) -> result<recipe>;
    void print_recipe(const recipe& input);
    auto build(const recipe& pkg_recipe, const recipe::build_env& env) -> result<void>;
    auto get_env_for_pkg(std::string_view name) -> recipe::build_env;
}    // namespace builder

using namespace std::literals;

namespace builder
{
    namespace fs   = std::filesystem;
    namespace proc = boost::process;

    void replace_special(std::string& input, const fs::path& install_dir)
    {
        constexpr auto install_dir_pattern = "${TCENV.INSTALL_DIR}";
        boost::replace_all(input, install_dir_pattern, install_dir.c_str());
    }

    auto run_child_proc(const fs::path& work_dir, const boost::process::environment& env, std::string exe,
                        std::vector<std::string> args)
    {
        fs::create_directories(work_dir);
        logger::debug("Run:", exe, "at", work_dir);
        for (auto&& arg: args)
        {
            logger::debug("   ", arg);
        }
        auto code = proc::system(exe, proc::args += args, proc::start_dir = work_dir.c_str(), proc::env = env);
        logger::debug("Exit code:", code);
        return code == 0;
    }

    void setup_gnu_recipe(recipe& target_recipe, toml::table& parsed_table)
    {
        std::vector<std::string> configureArgs;
        if (parsed_table.contains("configureArgs"))
        {
            for (auto&& item: *parsed_table["configureArgs"].as_array())
            {
                configureArgs.push_back(item.value_or("ERROR"s));
            }
        }

        std::vector<std::string> makeArgs;
        if (parsed_table.contains("makeArgs"))
        {
            for (auto&& item: *parsed_table["makeArgs"].as_array())
            {
                makeArgs.push_back(item.value_or("ERROR"s));
            }
        }
        // todo add to hash_data

        auto extr_dir = target_recipe.package_name + "-" + target_recipe.package_version;
        target_recipe.build_steps.emplace_back(
            [configureArgs = std::move(configureArgs), extr_dir](const recipe::build_env& env) -> result<void>
            {
                auto c_file                     = env.source_dir / extr_dir / "configure";
                auto c_env                      = env.variables;
                c_env["FORCE_UNSAFE_CONFIGURE"] = "1";
                auto c_args                     = std::vector<std::string>{"--prefix="s + env.install_dir.c_str()};
                std::copy(configureArgs.begin(), configureArgs.end(), std::back_inserter(c_args));

                if (!run_child_proc(env.build_dir, c_env, c_file.string(), c_args))
                {
                    return std::unexpected(error_t{"Process failed"});
                }

                return {};
            });

        target_recipe.build_steps.emplace_back(
            [makeArgs = std::move(makeArgs)](const recipe::build_env& env) -> result<void>
            {
                auto makeArgsCopy = makeArgs;
                for (auto& a: makeArgsCopy)
                {
                    replace_special(a, env.install_dir);
                    logger::info("makeArg", a);
                }

                auto c_args = std::vector<std::string>{"-j", "16"};
                std::copy(makeArgsCopy.begin(), makeArgsCopy.end(), std::back_inserter(c_args));

                if (!run_child_proc(env.build_dir, env.variables, proc::search_path("make").string(), c_args))
                {
                    return std::unexpected(error_t{"Process failed"});
                }
                return {};
            });

        target_recipe.build_steps.emplace_back(
            [makeArgs = std::move(makeArgs)](const recipe::build_env& env) -> result<void>
            {
                auto makeArgsCopy = makeArgs;
                for (auto& a: makeArgsCopy)
                {
                    replace_special(a, env.install_dir);
                    logger::info("makeArg", a);
                }
                auto c_args = std::vector<std::string>{"install"};
                std::copy(makeArgsCopy.begin(), makeArgsCopy.end(), std::back_inserter(c_args));

                if (!run_child_proc(env.build_dir, env.variables, proc::search_path("make").string(), c_args))
                {
                    return std::unexpected(error_t{"Process failed"});
                }
                return {};
            });
    }

    auto from_toml(std::string_view input) -> result<recipe>
    {
        auto parsed = toml::parse(input);

        // todo proper error handling
        auto res            = recipe{};
        res.package_name    = parsed["package"]["name"].value_or("ERROR"sv);
        res.package_version = parsed["package"]["version"].value_or("ERROR"sv);

        auto src_url = parsed["src"]["url"].value_or(""s);
        auto src_sha = parsed["src"]["sha256"].value_or(""s);
        if (!src_url.empty())
        {
            auto step = download_step{.url = src_url, .sha256 = src_sha};

            res.hash_data.append_range(step.get_sha_data());
            res.build_steps.push_back(std::move(step));
        }

        auto preset = parsed["src"]["preset"].value_or("none"sv);
        if (preset == "gnu")
        {
            setup_gnu_recipe(res, parsed);
        }

        auto env_node = parsed["propagates"]["env"];
        if (env_node.is_table())
        {
            for (auto&& [k, v]: *env_node.as_table())
            {
                if (v.is_array())
                {
                    for (auto&& vv: *v.as_array())
                    {
                        res.propagatesEnv.emplace_back(k.str(), vv.as_string()->get());
                    }
                }
                else
                {
                    logger::error("Key", k, " should contain array, but it has", v.type());
                    // todo make it better
                    return std::unexpected(error_t{"Parse error"});
                }
            }
        }
        else
        {
            logger::error("'propagates.env' is not a table, it is", env_node);
            // todo make it better
            return std::unexpected(error_t{"Parse error"});
        }

        return res;
    }

    void print_recipe(const recipe& input)
    {
        logger::debug(input.package_name, input.package_version);
        logger::debug("Propagates:");
        for (auto&& [var_name, var_val]: input.propagatesEnv)
        {
            logger::debug("\t", var_name, var_val);
        }
        logger::debug("Hash data");
        for (auto&& hash_part: input.hash_data)
        {
            logger::debug("\t", hash_part);
        }
    }

    constexpr auto succ_file = "_tcenv.success";

    void set_success(const fs::path& install_dir)
    {
        std::ofstream output((install_dir / succ_file).c_str());
    }

    auto is_success(const fs::path& install_dir) -> bool
    {
        return std::ifstream((install_dir / succ_file).c_str()).good();
    }

    auto build(const recipe& pkg_recipe, const recipe::build_env& env) -> result<void>
    {
        if (is_success(env.install_dir))
        {
            logger::info("Already built", pkg_recipe.package_name);
            return {};
        }

        for (const auto& step: pkg_recipe.build_steps)
        {
            TRY(step(env));
        }

        set_success(env.install_dir);
        return {};
    }

    auto get_env_for_pkg(std::string_view name) -> recipe::build_env
    {
        const auto root = fs::path("/tcroot");

        return recipe::build_env{.source_dir  = root / "src" / name,
                                 .build_dir   = root / "bld" / name,
                                 .install_dir = root / "store" / name,
                                 .variables   = boost::this_process::environment()};
    }
}    // namespace builder
