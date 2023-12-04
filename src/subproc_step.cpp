#include "subproc_step.hpp"

#include "store_util.hpp"

#include <boost/process.hpp>
#include <ranges>
#include <spdlog/fmt/ranges.h>
#include <spdlog/fmt/std.h>
#include <spdlog/spdlog.h>

namespace
{
    using std::filesystem::path;
    namespace proc = boost::process;

    auto replace_special(const std::string& input, const recipe::build_env& env) -> std::string
    {
        auto res = input;
        boost::replace_all(res, pattern::source_dir, env.source_dir.c_str());
        boost::replace_all(res, pattern::build_dir, env.build_dir.c_str());
        boost::replace_all(res, pattern::install_dir, env.install_dir.c_str());
        return res;
    }

    auto run_child_proc(const path& work_dir, const boost::process::environment& env, std::string exe,
                        std::vector<std::string> args)
    {
        create_directories(work_dir);
        spdlog::debug("Run: {} at {}", exe, work_dir);
        for (auto&& arg: args)
        {
            spdlog::debug("   {}", arg);
        }
        const auto code = proc::system(exe, proc::args += args, proc::start_dir = work_dir.c_str(), proc::env = env);
        spdlog::debug("Exit code: {}", code);
        return code == 0;
    }

    auto eval_rel_path(const rel_path& rp, const recipe::build_env& env) -> std::string
    {
        auto res = std::string{};
        switch (rp.base)
        {
            case rel_path::absolute:
                res = rp.value;
                break;
            case rel_path::source_dir:
                res = env.source_dir / rp.value;
                break;
            case rel_path::build_dir:
                res = env.build_dir / rp.value;
                break;
            case rel_path::install_dir:
                res = env.install_dir / rp.value;
                break;
            case rel_path::search:
                res = proc::search_path(rp.value.c_str()).string();
                break;
        }
        return res;
    }

}    // namespace

auto subproc_step::operator()(const recipe::build_env& env) const -> result<void>
{
    const auto exe_path = eval_rel_path(exe, env);
    const auto cwd      = eval_rel_path(work_dir, env);
    auto vars           = env.variables;
    for (auto&& [k, v]: aux_env)
    {
        vars[k] = replace_special(v, env);
    }
    auto exe_args = args;
    for (auto&& a: exe_args)
    {
        a = replace_special(a, env);
    }

    if (!run_child_proc(cwd, vars, exe_path, exe_args))
    {
        return std::unexpected(error_t{"Child process failed"});
    }
    return {};
}

auto subproc_step::get_sha_data() const -> std::vector<std::string>
{
    namespace views = std::ranges::views;

    auto res = std::vector<std::string>{};
    res.push_back(exe.value);

    // todo should we add work dir into the mix?
    // res.push_back(work_dir.value);

    std::ranges::copy(args, std::back_inserter(res));
    std::ranges::copy(aux_env
                          | views::transform([](auto&& pair) { return std::format("{}={}", pair.first, pair.second); }),
                      std::back_inserter(res));
    return res;
}
