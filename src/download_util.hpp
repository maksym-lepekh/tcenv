#ifndef DOWNLOAD_UTIL_HPP
#define DOWNLOAD_UTIL_HPP

#include "error.hpp"

#include <filesystem>

namespace download_util
{
    using std::filesystem::path;

    auto download(const std::string& url, const path& dest) -> result<path>;
}    // namespace download_util

#endif    // DOWNLOAD_UTIL_HPP
