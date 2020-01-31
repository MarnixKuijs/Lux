cmake_minimum_required(VERSION 3.8.0)
project(Lux)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(External/Glm)
add_subdirectory(External/Gsl)

add_subdirectory(External/Glfw)
set_target_properties(glfw
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/glfw/"
	LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/glfw/"
	ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/glfw/"
)

set(SRC_FILES
	./Lux/Source/glad.c
	./Lux/Source/Main.cpp
	./Lux/Source/Ray.cpp
	./Lux/Source/Camera.cpp
	./Lux/Source/Scene.cpp
	./Lux/Source/Mesh.cpp
)

add_executable(Lux ${SRC_FILES})

add_dependencies(Lux glfw)

target_include_directories(Lux
	PUBLIC ./Lux/Include/
	PUBLIC ./External/Glm/
	PUBLIC ./External/Gsl/include/
	PUBLIC ./External/Glfw/include/
)

target_link_directories(Lux
	PUBLIC ${CMAKE_BINARY_DIR}/bin/glfw/
)

target_link_libraries(Lux
	glfw3
)

target_compile_definitions(Lux
	PRIVATE NOMINMAX
)
