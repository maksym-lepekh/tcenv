cmake_minimum_required(VERSION 3.24)

project(tcenv VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

find_program(CCACHE_PROGRAM ccache)
if(CCACHE_PROGRAM)
    set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_PROGRAM}")
endif()

include(cmake/tcenv.cmake)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.70 REQUIRED COMPONENTS filesystem url)
tcenv_find_pkgconfig(libarchive)
tcenv_find_pkgconfig(libssl)

add_executable(tcenv
        src/main.cpp
        src/archive_util.hpp
        src/archive_util.cpp
        src/common.hpp
        src/download_util.hpp
        src/c_api.hpp
        src/download_util.cpp
        src/patcher.cpp
        src/patcher.hpp
        src/log.hpp
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
)

install(TARGETS tcenv DESTINATION bin)
