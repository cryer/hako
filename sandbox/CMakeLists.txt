cmake_minimum_required(VERSION 3.20)
project(sandbox C)
# recommend using vcpkg directly
find_package(raylib REQUIRED)

add_executable(${PROJECT_NAME} 
    src/main.c
)

target_link_libraries(${PROJECT_NAME} raylib)
