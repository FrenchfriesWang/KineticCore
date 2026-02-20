#version 460 core
layout (location = 0) in vec3 aPos; // 基础 Quad 顶点 (-0.5 到 0.5)
layout (location = 2) in vec4 aInstanceData; // xyz = 世界坐标偏移, w = 随机粗细

out vec2 TexCoord;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos; // [新增] 接收相机位置

// [修改] 大幅缩短基础 Y 轴长度，解决“面条感”
// 让雨滴回归短促、有力的拉丝感
const float BaseScaleX = 0.02;  // 更细的话可以0.015
const float BaseScaleY = 0.25;  // 从 0.8 降到 0.25

// [新增] 定义雨滴的物理下落方向 (暂时硬编码为垂直向下)
// 未来做“风”的时候，只需要把这个改成 uniform 变量即可动态调整风向！
const vec3 RainDirection = normalize(vec3(0.0, -1.0, 0.0));

void main()
{
    TexCoord = aPos.xy + 0.5;
    
    vec3 particleCenterWorldPos = aInstanceData.xyz;
    float randomScale = aInstanceData.w;

    // 计算最终尺寸
    float finalScaleX = BaseScaleX * randomScale; 
    float finalScaleY = BaseScaleY * (randomScale * 1.2); // 稍微拉大一点差异范围

    // --- [核心数学升级：速度对齐广告牌] ---

    // 1. 计算从粒子指向摄像机的向量 (视线方向的反方向)
    vec3 toCameraDir = normalize(cameraPos - particleCenterWorldPos);

    // 2. 确定粒子的“上”方向 (沿着雨滴下落轨迹的反方向)
    // 我们希望面片的 Y 轴沿着雨滴这一条线
    vec3 particleUp = -RainDirection; 

    // 3. 计算粒子的“右”方向 (关键一步！)
    // 利用叉乘：Right 向量必须同时垂直于“雨滴方向”和“视线方向”。
    // 这样能保证面片既沿着雨滴拉长，又尽量正对着摄像机。
    vec3 particleRight = normalize(cross(particleUp, toCameraDir));

    // [防御性编程] 极端情况处理：
    // 如果玩家正对着头顶垂直看 (toCameraDir 和 particleUp 平行)，叉乘结果长度会为 0。
    // 这时候我们需要给一个默认的 Right 方向 (比如世界坐标 X 轴) 防止消失。
    if (length(particleRight) < 0.001) {
        particleRight = vec3(1.0, 0.0, 0.0);
    }
    
    // 4. 构建最终顶点位置
    // 利用计算出的 Right 和 Up 轴构建局部坐标系
    vec3 finalVertexPos = particleCenterWorldPos 
                        + particleRight * aPos.x * finalScaleX 
                        + particleUp    * aPos.y * finalScaleY;

    gl_Position = projection * view * vec4(finalVertexPos, 1.0);
} 