cmake_minimum_required(VERSION 3.10)
project(KineticCore VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

# 显式定义文件列表 
set(SOURCE_FILES
    "src/main.cpp"
    "src/Shader.cpp"
    "src/Camera.cpp"
    "src/ParticleSystem.cpp"
    "vendor/glad/src/glad.c"
)

set(HEADER_FILES
    "include/Shader.h"
    "include/Camera.h"
    "include/Particle.h"
    "include/ParticleSystem.h"
    "vendor/glad/include/glad/glad.h"
    "vendor/glad/include/KHR/khrplatform.h"
    "vendor/stb_image/stb_image.h"
)

set(SHADER_FILES
    "assets/shaders/particle.vert"
    "assets/shaders/particle.frag"
    "assets/shaders/ground.vert"
    "assets/shaders/ground.frag"
)


# 使用 source_group 优化 IDE 视图
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Source Files" FILES ${SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Header Files" FILES ${HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Resources" FILES ${SHADER_FILES})

add_subdirectory(vendor/glfw)

# 构建指令
add_executable(${PROJECT_NAME}
    ${SOURCE_FILES}
    ${HEADER_FILES}
    ${SHADER_FILES}
)

# 包含路径管理
target_include_directories(${PROJECT_NAME} PRIVATE 
    "${CMAKE_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}/vendor/glad/include"
    "${CMAKE_SOURCE_DIR}/vendor/glfw/include"
    "${CMAKE_SOURCE_DIR}/vendor/glm"
    "${CMAKE_SOURCE_DIR}/vendor/stb_image"
)

# 链接库
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    OpenGL::GL
)

# 资源拷贝
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/assets"
    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying assets to output directory..."
)

# 调试路径设置
set_target_properties(${PROJECT_NAME} PROPERTIES 
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)