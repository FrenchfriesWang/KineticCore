#ifndef SHADER_H
#define SHADER_H

// Include Guard (防卫式声明)
// #ifndef ... #define ... #endif
// 防止头文件被多处引用时重复定义，导致编译错误。

#include <glad/glad.h> // 必须包含 GLAD 才能用 OpenGL 类型
#include <glm/glm.hpp>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    // 程序 ID (Program ID)
    // 类似于Windows 里的 "句柄" (Handle) 或者 Linux 的 "文件描述符" (fd)
    // 拿着这个 ID 就能指挥 GPU 里的这一套着色器程序。
    unsigned int ID;

    // 构造函数
    // 传入两个文件路径，它会自动读取、编译、链接
    Shader(const char* vertexPath, const char* fragmentPath);

    // 析构函数，对象销毁时自动清理 GPU 资源
    ~Shader();

    // 激活程序
    // 封装了 glUseProgram(ID)
    void use();

    // Uniform 工具函数 (后续会用到，先预留)
    // 用于 CPU 动态改变 GPU 里的变量（比如改变颜色、亮度）
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, const glm::vec2& value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setVec4(const std::string& name, const glm::vec4& value) const;

    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    // 私有函数：用于检查编译/链接是否出错
    // 这是一个很好的封装习惯：内部脏活累活不要暴露给外部
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif