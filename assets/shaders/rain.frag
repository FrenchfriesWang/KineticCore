#version 460 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 WorldPos; 

uniform sampler2D particleTexture;

uniform vec3 lightPos;          
uniform float coneHeight;       
uniform float coneBottomRadius; 
uniform float coneTopRadius;    
uniform vec3 lightColor;        
uniform float time;
void main()
{
    vec4 texColor = texture(particleTexture, TexCoord);
    if(texColor.a < 0.1) discard;

    // --- 1. 无分支数学遮罩 (依然完美判定内外) ---
    float h = lightPos.y - WorldPos.y;
    float distFromAxis = length(WorldPos.xz - lightPos.xz);
    float currentRadius = mix(coneTopRadius, coneBottomRadius, h / coneHeight);

    float verticalMask = step(0.0, h) * step(h, coneHeight); 
    float horizontalMask = 1.0 - smoothstep(currentRadius - 0.2, currentRadius, distFromAxis);
    float coneMask = verticalMask * horizontalMask;

    // --- 2. 绝对锁定：外面的雨 ---
    // 纯白 + 0.6透明度，保证外面的雨丝依然是细长、幽暗的。
    vec3 baseColor = vec3(1.0, 1.0, 1.0);
    float baseAlpha = texColor.a * 0.6; 

    // --- 3. 写实调优：内部的雨滴 (克制且自然) ---
    // 颜色：不再是暴力的纯色替换。现实中的雨滴在灯下依然是银白色的高光，只是稍微染上了路灯的环境色。
    // 我们把路灯的颜色按照 30% 的比例柔和地叠加到纯白雨滴上。
    vec3 insideColor = baseColor * (vec3(1.0) + lightColor * 0.6);
    
    // 透明度：解决“太粗”的元凶！
    // 把 5.0 倍的膨胀砍掉，只给 1.3 倍的微弱提亮。
    // 这样雨滴的渐变边缘依然柔和，绝对不会变胖，只会显得核心高光更锐利了一点点。
    float insideAlpha = texColor.a * 1.3; 

    // --- 4. 最终平滑混合 ---
    vec3 finalColor = mix(baseColor, insideColor, coneMask);
    float finalAlpha = min(mix(baseAlpha, insideAlpha, coneMask), 1.0);

    FragColor = vec4(finalColor, finalAlpha);
}