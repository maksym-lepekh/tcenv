cmake_minimum_required(VERSION 3.24)

project(tcenv VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 23)

set(Boost_USE_STATIC_LIBS ON)
find_package(Boost 1.70 REQUIRED COMPONENTS filesystem)

find_package(LibArchive REQUIRED)

add_executable(tcenv src/main.cpp
        src/archive_util.hpp
        src/archive_util.cpp
        src/common.hpp
        src/download_util.hpp)
target_link_options(tcenv PRIVATE "-static-libstdc++" "-static-libgcc")
target_include_directories(tcenv PRIVATE ${LibArchive_INCLUDE_DIRS})
target_link_libraries(tcenv PRIVATE
        Boost::headers
        Boost::filesystem
        -l:libarchive.a
)

install(TARGETS tcenv DESTINATION bin)
