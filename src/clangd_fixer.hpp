#ifndef TCENV_CLANGD_FIXER_HPP
#define TCENV_CLANGD_FIXER_HPP

#include <filesystem>
#include <string_view>
#include <unordered_map>

// Topic to read: https://github.com/llvm/llvm-project/issues/60027
#define FIX_CLANGD_MODULES          \
    namespace                       \
    {                               \
        [[maybe_unused]] void fix() \
        {                           \
            std::string_view sv;    \
        }                           \
    }

#endif    // TCENV_CLANGD_FIXER_HPP
