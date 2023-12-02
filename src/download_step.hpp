#ifndef DOWNLOAD_STEP_HPP
#define DOWNLOAD_STEP_HPP

#include "recipe_types.hpp"

struct download_step
{
    std::string url;
    std::string sha256;
    auto operator()(const recipe::build_env& env) const -> result<void>;
    auto get_sha_data() const -> std::vector<std::string>;
};

static_assert(std::is_convertible_v<download_step, recipe::build_step_fn>);

#endif // DOWNLOAD_STEP_HPP
