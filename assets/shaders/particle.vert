#version 460 core
layout (location = 0) in vec3 aPos;

void main()
{
    // 这里的 gl_Position 是 OpenGL 的内置变量
    // 它决定了 GPU 把这一把“沙子”（顶点）撒在屏幕的哪个位置
    gl_Position = vec4(aPos, 1.0);
}