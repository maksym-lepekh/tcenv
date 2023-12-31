module;
#include "control_flow.hpp"
#include <picosha2.h>

export module download_step;
import std;
import recipe_types;
import download_util;
import archive_util;
import logger;
import error;
import store_util;

export struct download_step
{
    std::string url;
    std::string sha256;
    auto operator()(const recipe::build_env& env) const -> result<void>;
    auto get_sha_data() const -> std::vector<std::string>;
};

static_assert(std::is_convertible_v<download_step, recipe::build_step_fn>);

using namespace std::literals;

namespace
{
    constexpr auto tag_fname = "_tcenv.digest";
}

auto download_step::operator()(const recipe::build_env& env) const -> result<void>
{
    if (file_marker::is_set(env.source_dir, tag_fname, sha256))
    {
        logger::info("Already downloaded", url);
    }

    if (create_directories(env.source_dir))
    {
        logger::info("Created dir:", env.source_dir);
    }

    auto downloaded = download_util::download(url, env.source_dir);
    if (downloaded)
    {
        logger::info("Downloaded:", downloaded.value());

        auto ifs = std::ifstream{downloaded.value(), std::ios::binary};
        auto first = std::istreambuf_iterator<char>(ifs);
        auto last = std::istreambuf_iterator<char>();

        auto f_sha = picosha2::hash256_hex_string(first, last);

        if (f_sha != sha256)
        {
            return std::unexpected(error_t{"Downloaded sha256: "s + f_sha + ", expected: " + sha256});
        }

        TRY(file_marker::set(env.source_dir, tag_fname, sha256));
        TRY(archive_util::extract(downloaded.value(), env.source_dir));
        return {};
    }
    return std::unexpected(downloaded.error());
}

auto download_step::get_sha_data() const -> std::vector<std::string>
{
    return { url, sha256 };
}
