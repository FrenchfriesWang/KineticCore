#version 460 core
layout (location = 0) in vec3 aPos; 
layout (location = 2) in vec4 aInstanceData; 

out vec2 TexCoord;
out vec3 WorldPos;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos; 

// [新增] 接收 C++ 传来的全局阵风矢量
uniform vec3 globalWind; 

const float BaseScaleX = 0.02;  
const float BaseScaleY = 0.25;  

void main()
{
    TexCoord = aPos.xy + 0.5;
    
    vec3 particleCenterWorldPos = aInstanceData.xyz;
    float randomScale = aInstanceData.w;

    float finalScaleX = BaseScaleX * randomScale; 
    float finalScaleY = BaseScaleY * (randomScale * 1.2); 

    // --- [核心升级：物理风力与微观湍流] ---
    
    // 1. 生成极微小的伪随机噪声 (基于粒子的世界坐标)
    // 制造 +/- 4.0 左右的微风扰动，打破绝对平行，消除虚线感！
    float noiseX = (fract(sin(dot(particleCenterWorldPos.xz, vec2(12.9898,78.233))) * 43758.5453) - 0.5) * 4.0;
    float noiseZ = (fract(sin(dot(particleCenterWorldPos.zx, vec2(39.346,11.135))) * 43758.5453) - 0.5) * 4.0;

    // 2. 计算这滴雨的真实物理轨迹矢量
    // 平均下落速度约为 -37.5 (来自 C++ 设置)。加上全局风力和它独有的微风扰动。
    vec3 actualVelocity = vec3(globalWind.x + noiseX, -37.5, globalWind.z + noiseZ);
    
    // 3. 归一化，得到真正的雨滴朝向
    vec3 trueRainDirection = normalize(actualVelocity);

    // --- [广告牌矩阵计算] ---
    
    vec3 toCameraDir = normalize(cameraPos - particleCenterWorldPos);

    // 让面片的 Y 轴严丝合缝地贴合物理轨迹的反方向
    vec3 particleUp = -trueRainDirection; 
    
    vec3 particleRight = normalize(cross(particleUp, toCameraDir));

    if (length(particleRight) < 0.001) {
        particleRight = vec3(1.0, 0.0, 0.0);
    }
    
    vec3 finalVertexPos = particleCenterWorldPos 
                        + particleRight * aPos.x * finalScaleX 
                        + particleUp    * aPos.y * finalScaleY;

    WorldPos = finalVertexPos; // 送给 frag 计算光锥
    gl_Position = projection * view * vec4(finalVertexPos, 1.0);
}