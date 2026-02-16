#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec4 ParticleColor;

uniform sampler2D particleTexture;

void main()
{
    // 采样纹理
    vec4 texColor = texture(particleTexture, TexCoord);
    
    // --- 关键修正：透明度剔除 ---
    // 如果纹理 alpha 值太低（透明区域），直接丢弃该像素
    // 解决“黑色方块”或“丑陋边缘”问题
    if(texColor.a < 0.1)
        discard;

    // 混合纹理颜色和粒子颜色
    FragColor = texColor * ParticleColor;
}