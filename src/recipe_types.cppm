module;
#include <vector>
#include <filesystem>
#include <boost/process.hpp>
#include <picosha2.h>

export module recipe_types;
import error;

export struct recipe
{
    std::string package_name;
    std::string package_version;
    std::vector<std::pair<std::string, std::string>> propagatesEnv;

    struct build_env
    {
        std::filesystem::path source_dir;
        std::filesystem::path build_dir;
        std::filesystem::path install_dir;
        boost::process::environment variables;
    };

    using build_step_fn = std::function<result<void>(const build_env&)>;

    std::vector<build_step_fn> build_steps = {};
    std::vector<std::string> hash_data;

    [[nodiscard]] auto get_sha256() const -> std::string;
};

auto recipe::get_sha256() const -> std::string
{
    auto hasher = picosha2::hash256_one_by_one{};
    for (auto&& val: hash_data)
    {
        hasher.process(val.begin(), val.end());
    }
    hasher.finish();
    return get_hash_hex_string(hasher);
}
