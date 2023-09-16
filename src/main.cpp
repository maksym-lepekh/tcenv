#include <fstream>
#include <string>
#include <boost/process.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <utility>
#include <archive.h>
#include "common.hpp"

import download_util;
import log;
import archive_util;

namespace fs = boost::filesystem;
namespace proc = boost::process;
using namespace std::literals;

void replace_special(std::string& input, const fs::path& install_dir)
{
    constexpr auto install_dir_pattern = "${TCENV.INSTALL_DIR}";
    boost::replace_all(input, install_dir_pattern, install_dir.c_str());
}

namespace store
{
    const auto root = fs::path("/tcroot");
    constexpr auto succ_file = "_tcenv.success";

    auto get_src_path(const std::string& package) -> fs::path
    {
        return root / "src" / package;
    }

    auto get_build_path(const std::string& package, unsigned stage) -> fs::path
    {
        return root / "bld" / std::to_string(stage) / package;
    }

    auto get_install_path(const std::string& package, unsigned stage) -> fs::path
    {
        return root / "store" / std::to_string(stage) / package;
    }

    void set_success(const fs::path& install_dir)
    {
        std::ofstream output((install_dir / succ_file).c_str());
    }

    bool is_success(const fs::path& install_dir)
    {
        return std::ifstream((install_dir / succ_file).c_str()).good();
    }
}

struct recipe
{
    std::string package_name;

    explicit recipe(std::string name): package_name(std::move(name)) {}
    [[nodiscard]] virtual bool build(const proc::environment& env, fs::path build_dir, fs::path install_dir) const = 0;
    virtual void extend_env(proc::native_environment& env, int stage) = 0;
    virtual ~recipe() = default;
};

struct gnu_build_recipe : recipe
{
    std::vector<std::string> configureArgs;
    std::vector<std::pair<std::string, std::string>> additionalEnv;
    std::vector<std::string> makeArgs;
    std::vector<std::pair<std::string, std::string>> propagatedRelEnv;
    bool doMakeInstall = true;
    bool doConfigure = true;
    bool runFromSourceDir = false;

    using recipe::recipe;

    [[nodiscard]] bool build(const proc::environment& env, fs::path build_dir, fs::path install_dir) const override
    {
        log::info("Building", package_name, "at", build_dir, "into", install_dir);
        if (store::is_success(install_dir))
        {
            log::info("already built");
            return true;
        }

        if (runFromSourceDir)
        {
            build_dir = store::get_src_path(package_name);
        }

        proc::system(proc::search_path("env"), proc::env = env);

        auto ret = proc::system(proc::search_path("mkdir"), "-p", build_dir, proc::env = env);
        if (ret != 0)
        {
            log::info("mkdir: non zero status", ret);
            return false;
        }

        if (doConfigure)
        {
            ret = proc::system(store::get_src_path(package_name) / "configure", "--prefix=" + install_dir.string(), proc::start_dir(build_dir), proc::args += configureArgs, proc::env = env, proc::env["FORCE_UNSAFE_CONFIGURE"] = "1");
            if (ret != 0)
            {
                log::info("configure: non zero status", ret);
                return false;
            }
        }

        auto makeArgsCopy = makeArgs;
        for (auto& a: makeArgsCopy)
        {
            replace_special(a, install_dir);
            log::info("makeArg", a);
        }

        ret = proc::system(proc::search_path("make"), "-j", "16", proc::args += makeArgsCopy, proc::start_dir(build_dir), proc::env = env);
        if (ret != 0)
        {
            log::info("make all: non zero status", ret);
            return false;
        }

        if (doMakeInstall)
        {
            ret = proc::system(proc::search_path("make"), "install", proc::args += makeArgsCopy, proc::start_dir(build_dir), proc::env = env);
            if (ret != 0)
            {
                log::info("make install: non zero status", ret);
                return false;
            }
        }

        store::set_success(install_dir);
        return true;
    }

    void extend_env(boost::process::native_environment &env, int stage) override {
        for (auto& [k, v]: propagatedRelEnv)
        {
            env[k] += (store::get_install_path(package_name, stage) / v).c_str();
        }
    }
};

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        log::info("wrong args");
        return EXIT_FAILURE;
    }

    log::info("libarchive:", archive_version_string());

    auto coreutils = gnu_build_recipe{"coreutils-9.3"};
    coreutils.propagatedRelEnv.emplace_back("PATH", "bin");

    auto sed = gnu_build_recipe{"sed-4.9"};
    sed.propagatedRelEnv.emplace_back("PATH", "bin");

    auto grep = gnu_build_recipe{"grep-3.11"};
    grep.propagatedRelEnv.emplace_back("PATH", "bin");

    auto make = gnu_build_recipe{"make-4.4"};
    make.propagatedRelEnv.emplace_back("PATH", "bin");

    auto glibc = gnu_build_recipe{"glibc-2.37"};
    glibc.propagatedRelEnv.emplace_back("PATH", "bin");
    glibc.propagatedRelEnv.emplace_back("CPATH", "include");
    glibc.propagatedRelEnv.emplace_back("LIBRARY_PATH", "lib");

    auto gawk = gnu_build_recipe{"gawk-5.2.2"};
    gawk.propagatedRelEnv.emplace_back("PATH", "bin");

    auto binutils = gnu_build_recipe{"binutils-2.41"};
    binutils.makeArgs.emplace_back("MAKEINFO=true");
    binutils.propagatedRelEnv.emplace_back("PATH", "bin");

    auto gcc = gnu_build_recipe("gcc-13.2.0");
    gcc.configureArgs.emplace_back("--disable-multilib");
    gcc.configureArgs.emplace_back("--enable-languages=c,c++");
    gcc.propagatedRelEnv.emplace_back("PATH", "bin");
    gcc.propagatedRelEnv.emplace_back("CPATH", "include/c++/13.2.0/x86_64-pc-linux-gnu");
    gcc.propagatedRelEnv.emplace_back("LIBRARY_PATH", "lib/gcc/x86_64-pc-linux-gnu/13.2.0");
    gcc.propagatedRelEnv.emplace_back("LIBRARY_PATH", "lib64");

    auto linux_headers = gnu_build_recipe("linux-5.15.123");
    linux_headers.doConfigure = false;
    linux_headers.doMakeInstall = false;
    linux_headers.makeArgs.emplace_back("headers_install");
    linux_headers.makeArgs.emplace_back("ARCH=x86_64");
    linux_headers.makeArgs.emplace_back("INSTALL_HDR_PATH=${TCENV.INSTALL_DIR}");
    linux_headers.propagatedRelEnv.emplace_back("CPATH", "include");

    auto gnu_toolchain = std::vector<recipe*>{
        &coreutils, &sed, &grep, &make, &gawk, &glibc, &binutils, &gcc, &linux_headers
    };

    auto build_env = boost::this_process::environment();
    auto stage = 0;

    auto build_list = gnu_toolchain;

    if (argv[1] == "s1"s)
    {
        build_env["PATH"].clear();
        for (auto& p: gnu_toolchain)
        {
            p->extend_env(build_env, 0);
        }
        stage = 1;

        build_list = std::vector<recipe*>{
                &coreutils, &sed, &grep, &make, &gawk, &glibc, &binutils, &gcc
        };
    }

    if (argv[1] == "test"s)
    {
        auto dest_dir = store::get_src_path("dummy");
        fs::create_directories(dest_dir);

        auto url = "https://ftp.gnu.org/gnu/sed/sed-4.9.tar.xz";
        auto dest = download_util::download(url, dest_dir);

        if (archive_util::extract(dest.value(), store::get_src_path("dummy")))
        {
            return EXIT_SUCCESS;
        }
        return EXIT_FAILURE;
    }

    for (auto& p: build_list)
    {
        if (!p->build(build_env,
                      store::get_build_path(p->package_name, stage),
                      store::get_install_path(p->package_name, stage)))
        {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
