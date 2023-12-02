#ifndef ARCHIVE_UTIL_HPP
#define ARCHIVE_UTIL_HPP

#include "error.hpp"

#include <filesystem>

namespace archive_util
{
    using std::filesystem::path;
    auto extract(const path& input, const path& dest) -> result<void>;
}    // namespace archive_util

#endif    // ARCHIVE_UTIL_HPP
