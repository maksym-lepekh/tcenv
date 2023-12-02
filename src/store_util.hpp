#ifndef STORE_UTIL_HPP
#define STORE_UTIL_HPP

#include "error.hpp"

#include <filesystem>
#include <string_view>

namespace file_marker
{
    using std::filesystem::path;

    auto is_set(const path& base_dir, std::string_view name, std::string_view val = {}) -> bool;

    auto clear(const path& base_dir, std::string_view name) -> result<>;

    auto set(const path& base_dir, std::string_view name, std::string_view val = {}) -> result<>;
}    // namespace file_marker

#endif    // STORE_UTIL_HPP
