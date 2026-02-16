#version 460 core
layout (location = 0) in vec3 aPos;      // 只有这个和 offset 是必须的
// layout (location = 1) in vec2 aTexCoord; // 暂时屏蔽它，防止 C++ 没传数据导致 bug
layout (location = 2) in vec3 offset;    // 实例位置

out vec2 TexCoord;
out vec4 ParticleColor;

uniform mat4 view;
uniform mat4 projection;

const float scaleX = 0.05; // 稍微调宽一点点，确保能看见
const float scaleY = 0.80; 

void main()
{
    TexCoord = aPos.xy + 0.5;
    ParticleColor = vec4(0.9, 0.95, 1.0, 0.8);

    // --- 修复逻辑开始 ---
    
    // 获取摄像机的 Right 向量 (保持横向面对摄像机)
    vec3 CameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    
    // 2. 强制 Up 向量为世界坐标的 Y 轴 (0, 1, 0)
    // 无论摄像机怎么抬头，雨滴都是垂直落下的
    vec3 WorldUp = vec3(0.0, 1.0, 0.0);

    // 构建位置
    // X 轴沿摄像机右方展开 (保证宽度可见)
    // Y 轴沿世界上方展开 (保证雨滴垂直)
    vec3 worldPos = offset 
                  + CameraRight * aPos.x * scaleX 
                  + WorldUp     * aPos.y * scaleY;


    gl_Position = projection * view * vec4(worldPos, 1.0);
}