#version 460 core
out vec4 FragColor;

in vec3 FragPos;
in vec2 TexCoords;
in vec3 Normal; 

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;
uniform sampler2D dispMap; 

uniform vec3 viewPos;
uniform float wetness; 
uniform float time; 

// --- [新增] 高质量的 2D 伪随机函数 ---
vec2 hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx  + 33.33);
    return fract((p3.xx + p3.yz) * p3.zy);
}

// --- [改进] 单层雨滴波纹生成器 ---
vec2 rippleLayer(vec2 uv, float t, float scale) {
    vec2 p = uv * scale;
    vec2 i = floor(p); // 网格 ID
    
    // 给水滴中心加上随机偏移，打破死板的网格感
    vec2 centerOffset = (hash22(i) - 0.5) * 0.4; 
    vec2 f = fract(p) - 0.5 - centerOffset; // 网格内的局部坐标
    
    // 随机的时间偏移
    float randTime = hash22(i + 1.414).x;
    float dropTime = fract(t + randTime);
    float dist = length(f);
    
    // 波纹环和淡出计算
    float ring = max(0.0, 1.0 - abs(dist - dropTime) * 12.0);
    float fade = (1.0 - dropTime) * smoothstep(0.5, 0.1, dist);
    
    // 返回 XY 偏移量
    return f * ring * fade * 0.4;
}

// --- [改进] 多层波纹混合 ---
vec3 getRainRippleNormal(vec2 uv, float t) {
    // 第一层波纹：较大，流速稍快
    vec2 offset1 = rippleLayer(uv, t * 1.5, 15.0);
    // 第二层波纹：较小，带有 UV 偏移，流速不同
    vec2 offset2 = rippleLayer(uv + vec2(0.23, 0.47), t * 1.2 + 0.5, 22.0);
    
    // 两层叠加，规律感彻底消失
    vec2 finalOffset = offset1 + offset2;
    return normalize(vec3(finalOffset.x, finalOffset.y, 1.0));
}

void main()
{
    // 1. 基础采样
    vec3 albedo = texture(albedoMap, TexCoords).rgb;
    float roughness = texture(roughnessMap, TexCoords).r;
    float ao = texture(aoMap, TexCoords).r;
    float disp = texture(dispMap, TexCoords).r; 

    vec3 normalMapValue = texture(normalMap, TexCoords).rgb;
    normalMapValue = normalize(normalMapValue * 2.0 - 1.0);

    // --- 2. 遮罩逻辑 (区分湿润区与深水区) ---
    float waterDepth = wetness - disp;
    
    // puddleMask：用于材质颜色的平滑过渡 (0.0 到 0.1)
    float puddleMask = smoothstep(0.0, 0.1, waterDepth);
    
    // rippleMask [核心修复]：只在水深大于 0.05 的“深水区”才允许有波纹
    // 避免波纹爬上石头边缘造成“穿模”错觉
    float rippleMask = smoothstep(0.05, 0.1, waterDepth);

    // 3. PBR 属性混合
    vec3 dampAlbedo = albedo * 0.6;      
    vec3 puddleAlbedo = albedo * 0.2;    
    vec3 finalAlbedo = mix(dampAlbedo, puddleAlbedo, puddleMask);

    float dampRoughness = roughness * 0.8;
    float finalRoughness = mix(dampRoughness, 0.02, puddleMask);

    // --- 4. 法线混合 (解决波纹穿模) ---
    vec3 rippleNormal = getRainRippleNormal(TexCoords, time);
    
    // 水面的纯平法线 (0,0,1)
    vec3 flatWaterNormal = vec3(0.0, 0.0, 1.0);
    // 真正的水面法线 = 平面 + 波纹 (受 rippleMask 严格限制)
    vec3 finalWaterNormal = normalize(mix(flatWaterNormal, rippleNormal, rippleMask));
    
    // 最终法线 = 混合石头凹凸与真正的水面
    vec3 finalNormalMapValue = normalize(mix(normalMapValue, finalWaterNormal, puddleMask));

    // 5. TBN 转换
    vec3 T = vec3(1.0, 0.0, 0.0);
    vec3 B = vec3(0.0, 0.0, 1.0);
    vec3 N_geom = vec3(0.0, 1.0, 0.0);
    mat3 TBN = mat3(T, B, N_geom);
    vec3 N = normalize(TBN * finalNormalMapValue); 

    // 6. 光照计算
    vec3 lightDir = normalize(vec3(0.2, 0.5, 0.5)); 
    vec3 lightColor = vec3(1.0, 1.0, 1.2) * 2.0;

    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(lightDir + V);

    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * finalAlbedo * lightColor;

    float shininess = (1.0 - finalRoughness) * 200.0; 
    float spec = pow(max(dot(N, H), 0.0), shininess);
    
    float F0 = mix(0.04, 0.02, puddleMask); 
    float fresnel = F0 + (1.0 - F0) * pow(1.0 - max(dot(V, N), 0.0), 5.0);
    
    // 积水处高光乘数放大
    vec3 specular = lightColor * spec * (fresnel + (1.0 - finalRoughness) * 0.5) * mix(1.0, 5.0, puddleMask); 

    // 环境光与假天空倒影
    vec3 baseAmbient = vec3(0.02) * finalAlbedo * ao;
    vec3 fakeSkyReflect = vec3(0.02, 0.03, 0.05) * puddleMask; 
    vec3 ambient = baseAmbient + fakeSkyReflect;

    vec3 color = ambient + diffuse + specular;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}