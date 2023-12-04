#ifndef RECIPE_TYPES_HPP
#define RECIPE_TYPES_HPP

#include "error.hpp"

#include <boost/process/environment.hpp>
#include <filesystem>
#include <picosha2.h>

struct recipe
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

    [[nodiscard]] auto get_hash() const -> std::string;
};

#endif    // RECIPE_TYPES_HPP
