#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec4 ParticleColor;

uniform sampler2D particleTexture;

void main()
{
    // [调试修改]
    // 1. 暂时屏蔽纹理采样（防止纹理加载失败导致黑屏）
    // 2. 暂时屏蔽 discard（防止全被当成透明的扔掉）
    
    // 强制输出：高亮白色，带一点点透明
    FragColor = vec4(1.0, 1.0, 1.0, 0.8); 
}