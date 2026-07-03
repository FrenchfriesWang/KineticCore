# KineticCore

KineticCore 是一个基于原生 C++ / OpenGL 实现的 3D 交互式雨夜场景项目，主要用于实践实时渲染管线、GLSL Shader、实例化渲染、纹理采样、GPU 端位移与运行时参数调节。

## 技术栈

- C++
- CMake
- OpenGL
- GLSL
- GLM
- GLFW
- GLAD
- stb_image
- miniaudio
- Dear ImGui

## 主要功能

- 3D 雨夜场景渲染
- 第一人称视角移动与交互
- 雨滴实例化渲染
- GPU 端雨滴位移
- 多通道贴图采样
- 湿润地面材质表现
- 环境音效与脚步声
- ImGui 运行时参数调节

## 项目截图

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/f0fea0fe-7eba-4a69-b61d-43ab4bf30582" />

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/866a43b4-7fb2-4825-81c8-97a7ac93ed23" />

<img width="1920" height="1080" alt="image" src="https://github.com/user-attachments/assets/15b4eb62-766a-469e-958a-6ce16addfd72" />


## 主技术要点

- 使用 GLSL 编写 Shader，实现雨滴、地面、光照等视觉效果
- 使用实例化渲染减少大量雨滴对象带来的 CPU 开销
- 将雨滴运动逻辑放到 GPU 端处理，减少 CPU 与 GPU 间频繁数据传输
- 使用 ImGui 暴露运行时参数，便于调试渲染效果

## 构建与运行

### 环境要求

- CMake >= 3.10
- 支持 C++17 的编译器
- OpenGL

项目依赖 GLFW、GLAD、GLM、stb_image、miniaudio、Dear ImGui，相关代码位于 `vendor/` 目录。

### 构建

```bash
mkdir build
cd build
cmake ..
cmake --build .
