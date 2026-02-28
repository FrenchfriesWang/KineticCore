#version 460 core
layout (location = 0) in vec3 aPos; 
layout (location = 2) in vec4 aInstanceData; // x,y,z 是初始降落点，w 是随机缩放

out vec2 TexCoord;
out vec3 WorldPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos; 
uniform vec3 windOffset;   // C++ 传来的真实风力累加位移 (决定位置)
uniform vec3 windVelocity; // C++ 传来的当前瞬间风速 (决定倾斜角度)
uniform float time; // CPU 每帧传进来的全局时间

const float BaseScaleX = 0.02;  
const float BaseScaleY = 0.25;  

void main()
{
    TexCoord = aPos.xy + 0.5;
    
    // 1. 获取这滴雨在游戏最初始的位置和缩放
    vec3 initialPos = aInstanceData.xyz;
    float randomScale = aInstanceData.w;

    // --- [新增 GPU 位移推演引擎] ---
    
    // 利用初始坐标生成一个稳定的伪随机下落速度 (模拟 C++ 里的 -30 到 -45)
    float randVal = fract(sin(dot(initialPos.xz, vec2(12.9898,78.233))) * 43758.5453);
    float randomSpeedY = mix(-30.0, -45.0, randVal);

    float noiseX = (fract(sin(dot(initialPos.xz, vec2(12.9898,78.233))) * 43758.5453) - 0.5) * 4.0;
    float noiseZ = (fract(sin(dot(initialPos.zx, vec2(39.346,11.135))) * 43758.5453) - 0.5) * 4.0;

    // 1. 算倾斜角度：只受当前瞬间的风速 (windVelocity) 影响
    vec3 actualVelocity = vec3(windVelocity.x + noiseX, randomSpeedY, windVelocity.z + noiseZ);
    
    // 2. 算绝对位置：雨滴自身下落位移 + C++ 精确累加的全局风向位移
    vec3 baseOffset = vec3(noiseX, randomSpeedY, noiseZ) * time; 
    vec3 totalOffset = baseOffset + windOffset; // 完美解耦！没有任何瞬移震荡！

    // 取模计算：让雨滴在 Y 轴 (-2 到 40) 和 XZ 轴 (摄像机周围 60x60) 无限循环！
    float currentY = mod(initialPos.y + totalOffset.y - (-2.0), 42.0) - 2.0;
    float currentX = mod(initialPos.x + totalOffset.x - cameraPos.x + 30.0, 60.0) + cameraPos.x - 30.0;
    float currentZ = mod(initialPos.z + totalOffset.z - cameraPos.z + 30.0, 60.0) + cameraPos.z - 30.0;

    // 得到当前帧真实的中心坐标
    vec3 currentCenterPos = vec3(currentX, currentY, currentZ);

    // --- [保留你原版的优秀视觉逻辑 (广告牌与粗细)] ---
    
    float finalScaleX = BaseScaleX * randomScale; 
    float finalScaleY = BaseScaleY * (randomScale * 1.2); 

    vec3 trueRainDirection = normalize(actualVelocity);
    vec3 toCameraDir = normalize(cameraPos - currentCenterPos);

    vec3 particleUp = -trueRainDirection; 
    vec3 particleRight = normalize(cross(particleUp, toCameraDir));

    if (length(particleRight) < 0.001) {
        particleRight = vec3(1.0, 0.0, 0.0);
    }
    
    // 用推演出的 currentCenterPos 来计算最终顶点
    vec3 finalVertexPos = currentCenterPos 
                        + particleRight * aPos.x * finalScaleX 
                        + particleUp    * aPos.y * finalScaleY;

    WorldPos = finalVertexPos; 
    gl_Position = projection * view * vec4(finalVertexPos, 1.0);
}