cmake_minimum_required(VERSION 3.20)
project(MyCraftPro LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 忽略警告 C4819
add_compile_options(/wd4819)

find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

# 包含头文件路径
include_directories(src)

# 递归查找所有源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(MyCraft ${SOURCES})

target_link_libraries(MyCraft PRIVATE glfw glm::glm glad::glad)

add_custom_command(TARGET MyCraft POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:MyCraft>/res
)