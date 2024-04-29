cmake_minimum_required(VERSION 3.20)
project(badbuffer)

add_executable(badbuffer "src/main.cpp")

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBPULSE REQUIRED IMPORTED_TARGET libpulse)
target_link_libraries(badbuffer PRIVATE PkgConfig::LIBPULSE)

find_package(ZLIB REQUIRED)
target_link_libraries(badbuffer PRIVATE ZLIB::ZLIB)
