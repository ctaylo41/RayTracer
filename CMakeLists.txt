cmake_minimum_required(VERSION 3.10.0)
project(RayTracer VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set (CMAKE_BUILD_TYPE Debug)
find_package(OpenGL REQUIRED)
find_package(PkgConfig REQUIRED)
find_package(glfw3 REQUIRED)
find_package(GLEW REQUIRED)
find_package(glm REQUIRED)

include_directories(${OPENGL_INCLUDE_DIRS} ${GLFW_INCLUDE_DIRS} ${GLEW_INCLUDE_DIRS} ${GLM_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/glad/include ${CMAKE_SOURCE_DIR}/include)

add_executable(RayTracer    ${CMAKE_SOURCE_DIR}/src/main.cpp 
                            ${CMAKE_SOURCE_DIR}/glad/src/glad.c 
                            ${CMAKE_SOURCE_DIR}/src/VAO.cpp 
                            ${CMAKE_SOURCE_DIR}/src/VBO.cpp 
                            ${CMAKE_SOURCE_DIR}/src/EBO.cpp 
                            ${CMAKE_SOURCE_DIR}/src/shader.cpp 
                            ${CMAKE_SOURCE_DIR}/src/camera.cpp)

if(NOT GLEW_LIBRARIES)
    set(GLEW_LIBRARIES /opt/homebrew/lib/libGLEW.dylib)
endif()
if(NOT GLFW_LIBRARIES)
    set(GLFW_LIBRARIES /opt/homebrew/opt/glfw/lib/libglfw.dylib)
endif()

target_link_libraries(RayTracer ${OPENGL_LIBRARIES} ${GLFW_LIBRARIES} ${GLEW_LIBRARIES})