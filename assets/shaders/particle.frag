#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
uniform sampler2D particleTexture;

void main()
{
    vec4 texColor = texture(particleTexture, TexCoord);
    
    // 剔除透明边缘，防止方块感
    if(texColor.a < 0.1) discard;

    // 纯白色拉丝雨滴，略带一点透明度 (0.6)，让它们交叠时更自然
    FragColor = vec4(1.0, 1.0, 1.0, texColor.a * 0.7); // 加入背景后可以0.06
}