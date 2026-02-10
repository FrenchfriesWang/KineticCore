cmake_minimum_required(VERSION 3.10)
project(KineticCore)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(vendor/glfw)

add_executable(
    KineticCore 
    src/main.cpp 
    src/Shader.cpp
    include/Shader.h
    vendor/glad/src/glad.c
)

target_include_directories(KineticCore PRIVATE 
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
)

target_link_libraries(KineticCore glfw OpenGL32)

add_custom_command(TARGET KineticCore POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:KineticCore>/assets"
    COMMENT "Copying assets to output directory..."
)

set_target_properties(KineticCore PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)