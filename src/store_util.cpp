#include "store_util.hpp"

#include <fstream>

using namespace std::literals;

namespace file_marker
{
    auto is_set(const path& base_dir, const std::string_view name, const std::string_view val) -> bool
    {
        if (auto sha_file = std::ifstream{base_dir / name})
        {
            auto content = ""s;
            sha_file >> content;
            return content == val;
        }
        return false;
    }

    auto clear(const path& base_dir, const std::string_view name) -> result<>
    {
        auto out_ec = std::error_code{};
        std::filesystem::remove(base_dir / name, out_ec);
        return out_ec ? std::unexpected{out_ec} : result<>{};
    }

    auto set(const path& base_dir, const std::string_view name, const std::string_view val) -> result<>
    {
        auto out_f = std::ofstream{base_dir / name};
        out_f << val;
        if (!out_f.good())
        {
            return std::unexpected{std::make_error_code(std::errc::io_error)};
        }
        return {};
    }
}    // namespace file_marker
