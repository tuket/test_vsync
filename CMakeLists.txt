cmake_minimum_required(VERSION 3.8)
set (CMAKE_CXX_STANDARD 17)
project(test_vsync)

add_subdirectory(libs/glad)
add_subdirectory(libs/glfw)
#add_subdirectory(libs/freeglut-3.0.0)

add_executable(test_glfw
	src/test_glfw.cpp
)
target_link_libraries(test_glfw glad glfw)

add_executable(test_glut3
	src/test_glut3.cpp
)
#target_link_libraries(test_glut3 glad freeglut30_static)