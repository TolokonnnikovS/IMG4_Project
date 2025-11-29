cmake_minimum_required(VERSION 3.15)
project(XmlToPngConverter)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Безопасные флаги компиляции
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# Поиск обязательных пакетов
find_package(PkgConfig REQUIRED)

# Поиск зависимостей
pkg_check_modules(PUGIXML REQUIRED pugixml)
pkg_check_modules(PNG REQUIRED libpng)
pkg_check_modules(FREETYPE REQUIRED freetype2)

message(STATUS "Found pugixml: ${PUGIXML_INCLUDE_DIRS}")
message(STATUS "Found libpng: ${PNG_INCLUDE_DIRS}")
message(STATUS "Found freetype: ${FREETYPE_INCLUDE_DIRS}")

# Исполняемый файл
add_executable(xml_to_png
    src/main.cpp
    src/xml_parser.cpp
    src/image_generator.cpp
    src/utils.cpp
)

# Подключение библиотек
target_link_libraries(xml_to_png PRIVATE 
    ${PUGIXML_LIBRARIES} 
    ${PNG_LIBRARIES} 
    ${FREETYPE_LIBRARIES}
)

target_include_directories(xml_to_png PRIVATE 
    ${PUGIXML_INCLUDE_DIRS} 
    ${PNG_INCLUDE_DIRS} 
    ${FREETYPE_INCLUDE_DIRS}
)

# Создаем папки для выходных данных
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/xml_png)

message(STATUS "Build configured successfully!")
message(STATUS "Output directory: ${CMAKE_CURRENT_BINARY_DIR}/xml_png")