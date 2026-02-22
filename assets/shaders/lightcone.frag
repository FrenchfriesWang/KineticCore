#version 460 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;

void main()
{
    // --- 1. 区分侧壁与顶盖 ---
    // 顶盖和侧壁的物理法线不同，如果强行用侧壁公式算顶盖，顶盖就会产生奇怪的阴影。
    float distToTop = lightPos.y - FragPos.y;
    float isTopCap = 1.0 - smoothstep(0.0, 0.05, distToTop); // 是否位于顶部盖子上

    // 侧壁的法线 (底3.0 - 顶0.3 = 2.7)
    vec2 xzDiff = FragPos.xz - lightPos.xz;
    if (length(xzDiff) < 0.0001) xzDiff = vec2(0.0001, 0.0);
    vec3 radialDir = normalize(vec3(xzDiff.x, 0.0, xzDiff.y));
    vec3 wallNormal = normalize(vec3(radialDir.x, 3.5 / 9.0, radialDir.z));

    vec3 viewDir = normalize(cameraPos - FragPos);
    
    // --- 2. 基础边缘羽化 ---
    // 只有侧壁才需要根据视角变透明，顶盖不参与边缘羽化，保证完整性
    float wallEdgeFade = abs(dot(viewDir, wallNormal));
    float edgeFade = mix(wallEdgeFade, 1.0, isTopCap);
    
    // 基础浓度 0.4，配合高度渐变和地面 0.5 米隐形防切线
    float fogBase = pow(edgeFade, 1.5) * clamp(FragPos.y / 9.0, 0.0, 1.0) * smoothstep(0.0, 0.5, FragPos.y) * 0.4;

    // --- [核心修复] 3D 指数发光球体 ---
    // 不再画死白的 2D 贴图！我们在光源原点生成一个指数衰减的 3D 发光体。
    // 它与顶板几何体相交时，会自然晕开一层极其平滑的强光。
    float distToLight = length(lightPos - FragPos);
    
    // exp 负指数：距离中心 0 时为 1，稍微偏离就平滑下降。
    // * 3.5 控制光球大小，* 1.5 控制中心刺眼程度。
    float coreGlow = exp(-distToLight * 3.5) * 1.5;

    // 最终合成：微弱雾气 + 核心刺眼光球
    float finalAlpha = fogBase + coreGlow;

    // --- 3. 消除色阶断层 ---
    float dither = fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453) * 0.02;
    finalAlpha += dither * finalAlpha; 

    FragColor = vec4(lightColor, finalAlpha);
}