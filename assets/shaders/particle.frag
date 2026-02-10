#version 460 core
out vec4 FragColor;

void main()
{
    // RGBA: 红, 绿, 蓝, 透明度
    // 这次我们换个颜色：青色 (Cyan)
    FragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
}