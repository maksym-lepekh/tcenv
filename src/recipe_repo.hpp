#ifndef RECIPE_REPO_HPP
#define RECIPE_REPO_HPP

#include "recipe_types.hpp"

struct recipe_repo
{
    auto init() -> bool;

    auto find_by_name(std::string_view pkg_name) -> std::optional<recipe>;

    std::unordered_map<std::string, recipe> recipes;
};

#endif    // RECIPE_REPO_HPP
