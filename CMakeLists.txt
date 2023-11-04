cmake_minimum_required(VERSION 3.28)

project(tcenv VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

# ccache
find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

# gsl
include(FetchContent)
FetchContent_Declare(GSL
        GIT_REPOSITORY "https://github.com/microsoft/GSL"
        GIT_TAG "v4.0.0"
        GIT_SHALLOW ON
)

# toml++
FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(GSL)
FetchContent_MakeAvailable(tomlplusplus)

# boost
set(Boost_USE_STATIC_RUNTIME ON)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.70 REQUIRED COMPONENTS filesystem url)

# pkg-config libs
include(cmake/tcenv.cmake)
tcenv_find_pkgconfig(libarchive)
tcenv_find_pkgconfig(libssl)

add_executable(tcenv
        src/main.cpp
        src/finally.hpp
        src/clangd_fixer.hpp
)

target_sources(tcenv PRIVATE
        FILE_SET CXX_MODULES FILES
        src/download_util.cppm
        src/logger.cppm
        src/c_api.cppm
        src/archive_util.cppm
        src/recipe.cppm
        src/recipe_repo.cppm
)

target_link_options(tcenv PRIVATE
        "-static"
)

target_link_libraries(tcenv
        Boost::headers
        Boost::filesystem
        Boost::url
        libssl::static
        libarchive::static
        Microsoft.GSL::GSL
        tomlplusplus::tomlplusplus
)

# use libc++
target_link_options(tcenv PRIVATE "-stdlib=libc++")
target_compile_options(tcenv PRIVATE "-stdlib=libc++")
target_link_libraries(tcenv c++abi)

install(TARGETS tcenv DESTINATION bin)
