#ifndef DOWNLOAD_UTIL_HPP
#define DOWNLOAD_UTIL_HPP

#include "error.hpp"
#include <filesystem>


namespace download_util
{
    using std::filesystem::path;
    auto download(const std::string& url, const path& dest) -> result<path>;
}

#endif // DOWNLOAD_UTIL_HPP
