#include "recipe_types.hpp"

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
