#include "recipe_types.hpp"

#include <array>

namespace
{
    // Base32 code from https://github.com/rhymu8354/Base64/blob/main/src/Base32.cpp
    constexpr char encoding_table[] = "abcdefghijklmnopqrstuvwxyz234567";

    template <typename T>
    std::string encode(const T& data, const bool omitPadding = false)
    {
        std::ostringstream output;
        size_t bits     = 0;
        uint32_t buffer = 0;

        for (const auto datum: data)
        {
            buffer <<= 8;
            buffer  += static_cast<uint32_t>(datum);
            bits    += 8;
            while (bits >= 5)
            {
                output << encoding_table[(buffer >> (bits - 5)) & 0x3f];
                buffer &= ~(0x1f << (bits - 5));
                bits   -= 5;
            }
        }

        if ((data.size() % 5) == 1)
        {
            buffer <<= 2;
            output << encoding_table[buffer & 0x1f];
            if (!omitPadding)
            {
                output << "======";
            }
        }
        if ((data.size() % 5) == 2)
        {
            buffer <<= 4;
            output << encoding_table[buffer & 0x1f];
            if (!omitPadding)
            {
                output << "====";
            }
        }
        else if ((data.size() % 5) == 3)
        {
            buffer <<= 1;
            output << encoding_table[buffer & 0x1f];
            if (!omitPadding)
            {
                output << "===";
            }
        }
        else if ((data.size() % 5) == 4)
        {
            buffer <<= 3;
            output << encoding_table[buffer & 0x1f];
            if (!omitPadding)
            {
                output << '=';
            }
        }
        return output.str();
    }
}    // namespace

auto recipe::get_hash() const -> std::string
{
    auto hasher = picosha2::hash256_one_by_one{};
    for (auto&& val: hash_data)
    {
        hasher.process(val.begin(), val.end());
    }
    hasher.finish();

    auto bytes = std::array<std::uint8_t, 32>{};
    hasher.get_hash_bytes(bytes.begin(), bytes.end());
    return encode(bytes, true).substr(0, 32);
}
