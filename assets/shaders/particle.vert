#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec4 aInstanceData; // xyz = pos, w = encoded state

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;

const float BaseScaleX = 0.05; 
const float BaseScaleY = 0.80; 

void main()
{
    TexCoord = aPos.xy + 0.5;
    
    vec3 offset = aInstanceData.xyz;
    float encodedState = aInstanceData.w;

    // --- 解码状态 ---
    // 如果 w > 0 是雨滴，否则是水花
    bool isRainState = encodedState > 0.0;
    
    // 获取水花的生命值 (0.0 ~ 0.15)
    // 如果是雨滴，这个值没意义
    float splashLife = abs(encodedState); 
    
    // --- 动态效果计算 ---
    
    // 1. 颜色与透明度
    vec4 color;
    if (isRainState) {
        color = vec4(0.9, 0.95, 1.0, 0.8); // 雨滴：固定高亮
    } else {
        // 水花动画核心：
        // Life 随着时间从 0.15 变到 0.0
        // 我们希望透明度随之降低：Life / 0.15
        float maxLife = 0.15;
        float alpha = clamp(splashLife / maxLife, 0.0, 1.0);
        
        // 让它稍微变得更透明一点，比如最大 0.6
        alpha *= 0.6; 
        
        color = vec4(1.0, 1.0, 1.0, alpha);
    }
    ParticleColor = color;

    // 2. 尺寸动态变化
    float currentScaleX, currentScaleY;
    
    if (isRainState) {
        currentScaleX = BaseScaleX;       // 0.05
        currentScaleY = BaseScaleY * 1.0; // 0.8 * 1.0
    } else {
        // 水花动画：
        // 随着生命流逝(splashLife变小)，让它稍微变大一点点(模拟扩散)
        // 比如从 0.25 扩散到 0.35
        float expandFactor = 1.0 + (0.15 - splashLife) * 2.0;
        
        currentScaleX = 0.25 * expandFactor; 
        currentScaleY = 0.25 * expandFactor; 
    }

    // --- 构建坐标 ---
    vec3 CameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 WorldUp = vec3(0.0, 1.0, 0.0);

    vec3 worldPos = offset 
                  + CameraRight * aPos.x * currentScaleX 
                  + WorldUp     * aPos.y * currentScaleY;

    gl_Position = projection * view * vec4(worldPos, 1.0);
}