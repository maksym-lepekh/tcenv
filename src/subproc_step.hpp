#ifndef SUBPROC_STEP_HPP
#define SUBPROC_STEP_HPP

#include "recipe_types.hpp"

#include <filesystem>
#include <string>
#include <vector>

struct rel_path
{
    enum relative_dir
    {
        absolute,
        source_dir,
        build_dir,
        install_dir,
        search
    };

    relative_dir base;
    std::filesystem::path value;
};

struct subproc_step
{
    rel_path exe;
    rel_path work_dir;
    std::vector<std::string> args;
    std::vector<std::pair<std::string, std::string>> aux_env;

    auto operator()(const recipe::build_env& env) const -> result<void>;
    auto get_sha_data() const -> std::vector<std::string>;
};

static_assert(std::is_convertible_v<subproc_step, recipe::build_step_fn>);

#endif    // SUBPROC_STEP_HPP
