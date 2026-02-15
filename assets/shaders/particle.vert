#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 offset;
uniform vec4 color;

void main()
{
    // [调试修改] 
    // 之前 width = 0.02 太细了，显卡画不出来。
    // 我们先改回 1.0 (不缩放)，确保能看见东西。
    float width = 1.0f;  
    float height = 5.0f; // 稍微拉长一点点
    
    vec3 scaledPos;
    scaledPos.x = aPos.x * width;
    scaledPos.y = aPos.y * height;
    scaledPos.z = aPos.z;

    vec3 finalPos = scaledPos + offset; 
    gl_Position = projection * view * vec4(finalPos, 1.0);
    
    TexCoord = aTexCoord;
    ParticleColor = color;
}