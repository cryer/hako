cmake_minimum_required(VERSION 3.20)

project(openGL CXX)

set(CMAKE_CXX_STANDARD 20) # 使用 C++20 标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 忽略警告 C4819
add_compile_options(/wd4819)

find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
find_package(ASSIMP REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(meshoptimizer CONFIG REQUIRED)

find_path(STB_INCLUDE_DIRS "stb_image.h") 
include_directories(${STB_INCLUDE_DIRS})

find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")
include_directories(${MINIAUDIO_INCLUDE_DIRS})

include(FetchContent)
FetchContent_Declare(
    ImGuiFileDialog
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG v0.6.8
)
FetchContent_MakeAvailable(ImGuiFileDialog)


add_executable(game 
	main.cpp
	Game.cpp
	model.cpp
	PlayerWeapon.cpp
	Renderer.cpp
	ResourceManager.cpp
	GazeMenu.cpp 
	DebugTerminal.cpp
	stb_image_impl.cpp
	AABB.cpp
	Frustum.cpp
	Level.cpp
	LevelEditor.cpp
	QuadTree.cpp
	AudioManager.cpp
	Terrain.cpp)

target_link_libraries(game PRIVATE glad::glad glfw glm::glm assimp::assimp imgui::imgui meshoptimizer::meshoptimizer  ImGuiFileDialog winmm)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

