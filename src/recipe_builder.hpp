#ifndef RECIPE_BUILDER_HPP
#define RECIPE_BUILDER_HPP

#include "recipe_types.hpp"

namespace builder
{
    auto from_toml(std::string_view input) -> result<recipe>;

    void print_recipe(const recipe& input);

    auto build(const recipe& pkg_recipe, const recipe::build_env& env) -> result<void>;

    auto get_env_for_pkg(const recipe& rec) -> recipe::build_env;
}    // namespace builder

#endif    // RECIPE_BUILDER_HPP
