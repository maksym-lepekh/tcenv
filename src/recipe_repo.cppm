export module recipe_repo;
import std;
import c_api;
import recipe_builder;
import recipe_types;

export struct recipe_repo
{
    auto init();

    auto find_by_name(std::string_view pkg_name) -> std::optional<recipe>;

    std::unordered_map<std::string, recipe> recipes;
};

module :private;

constexpr auto sed_4_9 = R"(
    [package]
    name = "sed"
    version = "4.9"

    [src]
    url = "https://ftp.gnu.org/gnu/sed/sed-4.9.tar.xz"
    preset = "gnu"

    [propagates.env]
    PATH = ['bin']
)";

auto recipe_repo::init()
{
    if (auto r = builder::from_toml(sed_4_9))
    {
        recipes["sed"] = r.value();
    }
    return true;
}

auto recipe_repo::find_by_name(std::string_view pkg_name) -> std::optional<recipe>
{
    if (recipes.contains(pkg_name.begin()))
    {
        return recipes[pkg_name.begin()];
    }
    return std::nullopt;
}

//    auto coreutils = gnu_build_recipe{"coreutils-9.3"};
//    coreutils.propagatedRelEnv.emplace_back("PATH", "bin");
//
//    auto sed = gnu_build_recipe{"sed-4.9"};
//    sed.propagatedRelEnv.emplace_back("PATH", "bin");
//
//    auto grep = gnu_build_recipe{"grep-3.11"};
//    grep.propagatedRelEnv.emplace_back("PATH", "bin");
//
//    auto make = gnu_build_recipe{"make-4.4"};
//    make.propagatedRelEnv.emplace_back("PATH", "bin");
//
//    auto glibc = gnu_build_recipe{"glibc-2.37"};
//    glibc.propagatedRelEnv.emplace_back("PATH", "bin");
//    glibc.propagatedRelEnv.emplace_back("CPATH", "include");
//    glibc.propagatedRelEnv.emplace_back("LIBRARY_PATH", "lib");
//
//    auto gawk = gnu_build_recipe{"gawk-5.2.2"};
//    gawk.propagatedRelEnv.emplace_back("PATH", "bin");
//
//    auto binutils = gnu_build_recipe{"binutils-2.41"};
//    binutils.makeArgs.emplace_back("MAKEINFO=true");
//    binutils.propagatedRelEnv.emplace_back("PATH", "bin");
//
//    auto gcc = gnu_build_recipe("gcc-13.2.0");
//    gcc.configureArgs.emplace_back("--disable-multilib");
//    gcc.configureArgs.emplace_back("--enable-languages=c,c++");
//    gcc.propagatedRelEnv.emplace_back("PATH", "bin");
//    gcc.propagatedRelEnv.emplace_back("CPATH", "include/c++/13.2.0/x86_64-pc-linux-gnu");
//    gcc.propagatedRelEnv.emplace_back("LIBRARY_PATH", "lib/gcc/x86_64-pc-linux-gnu/13.2.0");
//    gcc.propagatedRelEnv.emplace_back("LIBRARY_PATH", "lib64");
//
//    auto linux_headers          = gnu_build_recipe("linux-5.15.123");
//    linux_headers.doConfigure   = false;
//    linux_headers.doMakeInstall = false;
//    linux_headers.makeArgs.emplace_back("headers_install");
//    linux_headers.makeArgs.emplace_back("ARCH=x86_64");
//    linux_headers.makeArgs.emplace_back("INSTALL_HDR_PATH=${TCENV.INSTALL_DIR}");
//    linux_headers.propagatedRelEnv.emplace_back("CPATH", "include");
