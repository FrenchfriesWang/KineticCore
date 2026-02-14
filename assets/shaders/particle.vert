#version 460 core
layout (location = 0) in vec3 aPos;

// 【新增】接收三大矩阵
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    // 矩阵乘法顺序：投影 * 视图 * 模型 * 顶点
    // 注意：顺序不能反！
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}