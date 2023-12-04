#include "recipe_builder.hpp"

#include "control_flow.hpp"
#include "download_step.hpp"
#include "store_util.hpp"
#include "subproc_step.hpp"

#include <boost/process.hpp>
#include <spdlog/fmt/std.h>
#include <spdlog/spdlog.h>
#include <toml++/toml.hpp>

using namespace std::literals;

namespace builder
{
    namespace fs   = std::filesystem;
    namespace proc = boost::process;
    using std::filesystem::path;

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

        auto extr_dir = target_recipe.package_name + "-" + target_recipe.package_version;

        // configure
        auto conf_step     = subproc_step{};
        conf_step.exe      = {rel_path::source_dir, path{extr_dir} / "configure"};
        conf_step.work_dir = {rel_path::build_dir, {}};
        conf_step.aux_env.emplace_back("FORCE_UNSAFE_CONFIGURE", "1");
        conf_step.args = configureArgs;
        conf_step.args.push_back(std::format("--prefix={}", pattern::install_dir));
        std::ranges::copy(conf_step.get_sha_data(), std::back_inserter(target_recipe.hash_data));
        target_recipe.build_steps.push_back(std::move(conf_step));

        // make
        auto make_step     = subproc_step{};
        make_step.exe      = {rel_path::search, "make"};
        make_step.work_dir = {rel_path::build_dir, {}};
        make_step.args     = {"-j", "16"};
        std::ranges::copy(makeArgs, std::back_inserter(make_step.args));
        std::ranges::copy(make_step.get_sha_data(), std::back_inserter(target_recipe.hash_data));
        target_recipe.build_steps.push_back(make_step);

        // make install, reuse struct above
        make_step.args = {"install"};
        std::ranges::copy(make_step.get_sha_data(), std::back_inserter(target_recipe.hash_data));
        target_recipe.build_steps.push_back(std::move(make_step));
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
            auto step     = download_step{.url = src_url, .sha256 = src_sha};
            auto sha_data = step.get_sha_data();
            res.hash_data.insert(res.hash_data.end(), sha_data.begin(), sha_data.end());
            res.build_steps.push_back(std::move(step));
        }

        if (auto preset = parsed["src"]["preset"].value_or("none"sv); preset == "gnu")
        {
            setup_gnu_recipe(res, parsed);
        }

        if (auto env_node = parsed["propagates"]["env"]; env_node.is_table())
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
                    spdlog::error("Key {} should contain array, but it has {}", k, v.type());
                    // todo make it better
                    return std::unexpected(error_t{"Parse error"});
                }
            }
        }
        else
        {
            spdlog::error("'propagates.env' is not a table, it is {}", env_node);
            // todo make it better
            return std::unexpected(error_t{"Parse error"});
        }

        return res;
    }

    void print_recipe(const recipe& input)
    {
        spdlog::debug("{} {}", input.package_name, input.package_version);
        spdlog::debug("Recipe hash: {}", input.get_hash());
        spdlog::debug("Propagates:");
        for (auto&& [var_name, var_val]: input.propagatesEnv)
        {
            spdlog::debug("    {}={}", var_name, var_val);
        }
        spdlog::debug("Hash data");
        for (auto&& hash_part: input.hash_data)
        {
            spdlog::debug("    {}", hash_part);
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
            spdlog::info("Already built: {}", pkg_recipe.package_name);
            return {};
        }

        for (const auto& step: pkg_recipe.build_steps)
        {
            TRY(step(env));
        }

        set_success(env.install_dir);
        return {};
    }

    auto get_env_for_pkg(const recipe& rec) -> recipe::build_env
    {
        const auto root     = path("/tcroot");
        const auto dir_name = std::format("{}-{}-{}", rec.get_hash(), rec.package_name, rec.package_version);

        return recipe::build_env{.source_dir  = root / "src" / dir_name,
                                 .build_dir   = root / "bld" / dir_name,
                                 .install_dir = root / "store" / dir_name,
                                 .variables   = boost::this_process::environment()};
    }
}    // namespace builder
