cmake_minimum_required(VERSION 3.27)

project(tcenv VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)

# ccache
option(USE_CCACHE "Enable ccache in the build" ON)
find_program(CCACHE_PROGRAM ccache)
if (CCACHE_PROGRAM AND USE_CCACHE)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

include(FetchContent)
FetchContent_Declare(GSL
        GIT_REPOSITORY "https://github.com/microsoft/GSL"
        GIT_TAG "v4.0.0"
        GIT_SHALLOW ON
)

FetchContent_Declare(
        tomlplusplus
        GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
        GIT_TAG v3.4.0
)

FetchContent_Declare(
        digestpp
        GIT_REPOSITORY https://github.com/kerukuro/digestpp.git
        GIT_TAG ebb699402c244e22c3aff61d2239bcb2e87b8ef8
)

FetchContent_Declare(
        PicoSHA2
        GIT_REPOSITORY https://github.com/okdshin/PicoSHA2.git
        GIT_TAG v1.0.0
)

FetchContent_Declare(
        spdlog
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.12.0
)

FetchContent_MakeAvailable(GSL tomlplusplus digestpp PicoSHA2 spdlog)

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
        src/control_flow.hpp
        src/archive_util.cpp
        src/download_step.cpp
        src/download_util.cpp
        src/store_util.cpp
        src/recipe_types.cpp
        src/recipe_repo.cpp
        src/recipe_builder.cpp
        src/subproc_step.hpp
        src/subproc_step.cpp
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
        spdlog::spdlog
)

target_include_directories(tcenv PRIVATE
        ${digestpp_SOURCE_DIR}
        ${picosha2_SOURCE_DIR}
)

install(TARGETS tcenv DESTINATION bin)
