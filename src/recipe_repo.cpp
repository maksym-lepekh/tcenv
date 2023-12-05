#include "recipe_repo.hpp"

#include "recipe_builder.hpp"

#include <spdlog/spdlog.h>

constexpr auto sed_4_9 = R"(
    [package]
    name = "sed"
    version = "4.9"

    [src]
    url = "https://ftp.gnu.org/gnu/sed/sed-4.9.tar.xz"
    sha256 = "6e226b732e1cd739464ad6862bd1a1aba42d7982922da7a53519631d24975181"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
)";

constexpr auto coreutils_9_4 = R"(
    [package]
    name = "coreutils"
    version = "9.4"

    [src]
    url = "https://ftp.gnu.org/gnu/coreutils/coreutils-9.4.tar.xz"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
)";

constexpr auto grep_3_11 = R"(
    [package]
    name = "grep"
    version = "3.11"

    [src]
    url = "https://ftp.gnu.org/gnu/grep/grep-3.11.tar.xz"
    sha256 = "1db2aedde89d0dea42b16d9528f894c8d15dae4e190b59aecc78f5a951276eab"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
)";

constexpr auto make_4_4 = R"(
    [package]
    name = "make"
    version = "4.4"

    [src]
    url = "https://ftp.gnu.org/gnu/make/make-4.4.tar.lz"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
)";

constexpr auto glibc_2_37 = R"(
    [package]
    name = "glibc"
    version = "2.37"

    [src]
    url = "https://ftp.gnu.org/gnu/glibc/glibc-2.37.tar.xz"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
    CPATH = ['include']
    LIBRARY_PATH = ['lib']
)";

constexpr auto gawk_5_2_2 = R"(
    [package]
    name = "gawk"
    version = "5.2.2"

    [src]
    url = "https://ftp.gnu.org/gnu/gawk/gawk-5.2.2.tar.xz"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
)";

constexpr auto binutils_2_41 = R"(
    [package]
    name = "binutils"
    version = "2.41"

    [src]
    url = "https://ftp.gnu.org/gnu/binutils/binutils-2.41.tar.xz"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
)";

constexpr auto gcc_13_2_0 = R"(
    [package]
    name = "gcc"
    version = "13.2.0"

    [src]
    url = "https://ftp.gnu.org/gnu/gcc/gcc-13.2.0/gcc-13.2.0.tar.xz"
    preset = "gnu"

    [gnu]
    configureArgs = ['--disable-multilib', '--enable-languages=c,c++']

    [propagates.env]
    PATH = ['bin']
    CPATH = ['include/c++/13.2.0/x86_64-pc-linux-gnu']
    LIBRARY_PATH = ['lib/gcc/x86_64-pc-linux-gnu/13.2.0', 'lib64']
)";

auto recipe_repo::init() -> bool
{
    for (auto&& recipe_str:
         {sed_4_9, coreutils_9_4, grep_3_11, make_4_4, glibc_2_37, gawk_5_2_2, binutils_2_41, gcc_13_2_0})
    {
        if (const auto r = builder::from_toml(recipe_str))
        {
            recipes.emplace(r->package_name, std::move(*r));
        }
    }

    return true;
}

auto recipe_repo::find_by_name(const std::string_view pkg_name) -> std::optional<recipe>
{
    if (recipes.contains(pkg_name.begin()))
    {
        return recipes[pkg_name.begin()];
    }
    return std::nullopt;
}

void recipe_repo::print_list()
{
    for (auto&& [k, v]: recipes)
    {
        spdlog::debug("{:<12}-> {:<12} {:<6} {}", k, v.package_name, v.package_version, v.get_hash());
    }
}

//
//    auto linux_headers          = gnu_build_recipe("linux-5.15.123");
//    linux_headers.doConfigure   = false;
//    linux_headers.doMakeInstall = false;
//    linux_headers.makeArgs.emplace_back("headers_install");
//    linux_headers.makeArgs.emplace_back("ARCH=x86_64");
//    linux_headers.makeArgs.emplace_back("INSTALL_HDR_PATH=${TCENV.INSTALL_DIR}");
//    linux_headers.propagatedRelEnv.emplace_back("CPATH", "include");
