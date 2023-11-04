module;
#include "clangd_fixer.hpp"
#include "control_flow.hpp"

#include <boost/algorithm/string.hpp>
#include <boost/process.hpp>
#include <expected>
#include <filesystem>
#include <string_view>
#include <system_error>
#include <toml++/toml.hpp>
#include <vector>

export module recipe;
import download_util;
import archive_util;
import error;
import logger;
FIX_CLANGD_MODULES;

using namespace std::literals;

export struct recipe
{
    std::string package_name;
    std::string package_version;
    std::vector<std::pair<std::string, std::string>> propagatesEnv;

    struct build_env
    {
        std::filesystem::path source_dir;
        std::filesystem::path build_dir;
        std::filesystem::path install_dir;
        boost::process::environment variables;
    };

    using build_step_fn = std::function<result<void>(const build_env&)>;

    std::vector<build_step_fn> build_steps = {};
    std::vector<std::string> hash_data;
};

namespace recipe_builder
{
    namespace fs   = std::filesystem;
    namespace proc = boost::process;

    auto download_step(const recipe::build_env& env, const std::string& url) -> result<void>
    {
        fs::create_directories(env.source_dir);
        logger::info("Created dir:", env.source_dir);
        auto downloaded = download_util::download(url, env.source_dir);
        if (downloaded)
        {
            logger::info("Downloaded:", downloaded.value());
            TRY(archive_util::extract(downloaded.value(), env.source_dir));
            return {};
        }
        return std::unexpected(downloaded.error());
    }

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
        for (const auto& a: args)
        {
            logger::debug("   ", a);
        }
        auto code = proc::system(exe, proc::args += args, proc::start_dir = work_dir.c_str(), proc::env = env);
        logger::debug("Exit code:", code);
        return code == 0;
    }

    void setup_gnu_recipe(recipe& r, toml::table& t)
    {
        std::vector<std::string> configureArgs;
        if (t.contains("configureArgs"))
        {
            for (auto&& item: *t["configureArgs"].as_array())
            {
                configureArgs.push_back(item.value_or("ERROR"s));
            }
        }

        std::vector<std::string> makeArgs;
        if (t.contains("makeArgs"))
        {
            for (auto&& item: *t["makeArgs"].as_array())
            {
                makeArgs.push_back(item.value_or("ERROR"s));
            }
        }
        // todo add to hash_data

        auto extr_dir = r.package_name + "-" + r.package_version;
        r.build_steps.emplace_back(
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

        r.build_steps.emplace_back(
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

        r.build_steps.emplace_back(
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

    export auto from_toml(std::string_view input) -> result<recipe>
    {
        // logger::info("Called with input", input);
        auto parsed = toml::parse(input);
        // logger::info("Done parsing", parsed);

        // todo proper error handling
        auto res            = recipe{};
        res.package_name    = parsed["package"]["name"].value_or("ERROR"sv);
        res.package_version = parsed["package"]["version"].value_or("ERROR"sv);

        auto url = parsed["src"]["url"].value_or(""s);
        if (!url.empty())
        {
            res.hash_data.push_back(url);
            res.build_steps.emplace_back([url](auto& env) { return download_step(env, url); });
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

    export void print_recipe(const recipe& r)
    {
        logger::debug(r.package_name, r.package_version);
        logger::debug("Propagates:");
        for (auto&& a: r.propagatesEnv)
        {
            logger::debug("\t", a.first, a.second);
        }
        logger::debug("Hash data");
        for (auto&& h: r.hash_data)
        {
            logger::debug("\t", h);
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

    export auto build(const recipe& pkg_recipe, const recipe::build_env& env) -> result<void>
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

    export auto get_env_for_pkg(std::string_view name) -> recipe::build_env
    {
        const auto root = fs::path("/tcroot");

        return recipe::build_env{.source_dir  = root / "src" / name,
                                 .build_dir   = root / "bld" / name,
                                 .install_dir = root / "store" / name,
                                 .variables   = boost::this_process::environment()};
    }
}    // namespace recipe_builder
