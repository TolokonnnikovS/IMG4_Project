cmake_minimum_required(VERSION 3.15)
project(FbtToPngConverter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# pugixml
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG v1.14
)

# libpng - НОВАЯ ВЕРСИЯ 1.6.40
FetchContent_Declare(
    libpng
    URL https://download.sourceforge.net/libpng/libpng-1.6.40.tar.gz
)

# freetype
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG VER-2-13-2
)

# argparse - НОВАЯ БИБЛИОТЕКА ДЛЯ АРГУМЕНТОВ КОМАНДНОЙ СТРОКИ
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v2.9
)

FetchContent_MakeAvailable(pugixml libpng freetype argparse)

add_executable(fbt_to_png
    src/main.cpp
    src/xml_parser.cpp
    src/image_generator.cpp
    src/utils.cpp
)

target_include_directories(fbt_to_png PRIVATE
    src
    ${pugixml_SOURCE_DIR}/src
    ${libpng_SOURCE_DIR}
    ${freetype_SOURCE_DIR}/include
    ${argparse_SOURCE_DIR}/include
)

target_link_libraries(fbt_to_png PRIVATE
    pugixml
    png
    freetype
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/xml_png)