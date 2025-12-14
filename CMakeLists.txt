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

# freetype
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
    GIT_TAG VER-2-13-2
)

# argparse
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
    GIT_TAG v2.9
)

# stb (Новая библиотека для изображений)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)

FetchContent_MakeAvailable(pugixml freetype argparse stb)

add_executable(fbt_to_png
    src/main.cpp
    src/xml_parser.cpp
    src/image_generator.cpp
    src/utils.cpp
)

target_include_directories(fbt_to_png PRIVATE
    src
    ${pugixml_SOURCE_DIR}/src
    ${freetype_SOURCE_DIR}/include
    ${argparse_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}  # stb_image_write.h здесь
)

target_link_libraries(fbt_to_png PRIVATE
    pugixml
    freetype
)

# Копируем stb_image_write.h в нужную папку (опционально)
file(COPY ${stb_SOURCE_DIR}/stb_image_write.h DESTINATION ${CMAKE_CURRENT_SOURCE_DIR}/src)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/xml_png)