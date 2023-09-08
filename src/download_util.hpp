//
// Created by maxl on 11.08.23.
//

#ifndef TCENV_DOWNLOAD_UTIL_HPP
#define TCENV_DOWNLOAD_UTIL_HPP

#include <boost/filesystem.hpp>
#include <boost/beast/core/error.hpp>
#include <string>
#include <expected>


namespace download_util
{
    namespace fs = boost::filesystem;

    auto download(const std::string& url, const fs::path& dest) -> std::expected<fs::path, boost::beast::error_code>;
}

#endif //TCENV_DOWNLOAD_UTIL_HPP
