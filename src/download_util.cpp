#include "download_util.hpp"

#include <boost/url.hpp>
#include <httplib.h>
#include <spdlog/fmt/std.h>
#include <spdlog/spdlog.h>

namespace download_util
{
    auto download(const std::string& url, const path& dest) -> result<path>
    {
        auto parsed_url = boost::urls::parse_uri(url);
        auto host       = std::string(parsed_url->encoded_host());
        auto resource   = std::string(parsed_url->encoded_resource());
        auto file_name  = std::string(parsed_url->encoded_path());
        if (file_name.empty())
        {
            file_name = "file";
        }
        if (file_name.contains('/'))
        {
            file_name = file_name.substr(file_name.find_last_of('/') + 1);
        }
        auto dest_file = dest / file_name;

        spdlog::debug("host = {}", host);
        spdlog::debug("resource = {}", resource);
        spdlog::debug("dest_file = {}", dest_file);

        auto stream = std::ofstream{dest_file};
        auto client = httplib::Client(host);
        auto res    = client.Get(resource,
                                 [&](const char* data, size_t data_length)
                                 {
                                  stream.write(data, data_length);
                                  return true;
                              });

        if (res->status != 200)
        {
            return std::unexpected(to_string(res.error()));
        }

        return dest_file;
    }
}    // namespace download_util
