#include "download_step.hpp"

#include "archive_util.hpp"
#include "control_flow.hpp"
#include "download_util.hpp"
#include "store_util.hpp"

#include <fstream>
#include <picosha2.h>
#include <spdlog/fmt/std.h>
#include <spdlog/spdlog.h>

namespace
{
    constexpr auto tag_fname = "_tcenv.digest";
}

auto download_step::operator()(const recipe::build_env& env) const -> result<void>
{
    if (file_marker::is_set(env.source_dir, tag_fname, sha256))
    {
        spdlog::info("Already downloaded: {}", url);
    }

    if (create_directories(env.source_dir))
    {
        spdlog::info("Created dir: {}", env.source_dir);
    }

    auto downloaded = download_util::download(url, env.source_dir);
    if (downloaded)
    {
        spdlog::info("Downloaded: {}", downloaded.value());

        auto ifs   = std::ifstream{downloaded.value(), std::ios::binary};
        auto first = std::istreambuf_iterator(ifs);
        auto last  = std::istreambuf_iterator<char>();

        if (auto f_sha = picosha2::hash256_hex_string(first, last); f_sha != sha256)
        {
            return std::unexpected(error_t{std::format("Downloaded sha256: {}, expected: {}", f_sha, sha256)});
        }

        TRY(file_marker::set(env.source_dir, tag_fname, sha256));
        TRY(archive_util::extract(downloaded.value(), env.source_dir));
        return {};
    }
    return std::unexpected(downloaded.error());
}

auto download_step::get_sha_data() const -> std::vector<std::string>
{
    return {url, sha256};
}
