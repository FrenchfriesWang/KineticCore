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
uniform vec3 lightPos;
uniform vec3 lightColor;

// --- 高质量的 2D 伪随机函数 ---
vec2 hash22(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * vec3(.1031, .1030, .0973));
    p3 += dot(p3, p3.yzx  + 33.33);
    return fract((p3.xx + p3.yz) * p3.zy);
}

// --- 单层雨滴波纹生成器 ---
vec2 rippleLayer(vec2 uv, float t, float     scale) {
    vec2 p = uv * scale;
    vec2 i = floor(p); 
    
    vec2 centerOffset = (hash22(i) - 0.5) * 0.4; 
    vec2 f = fract(p) - 0.5 - centerOffset; 
    
    float randTime = hash22(i + 1.414).x;
    float dropTime = fract(t + randTime);
    float dist = length(f);
    
    float ring = max(0.0, 1.0 - abs(dist - dropTime) * 12.0);
    float fade = (1.0 - dropTime) * smoothstep(0.5, 0.1, dist);
    
    return f * ring * fade * 0.4;
}

// --- 多层波纹混合 ---
vec3 getRainRippleNormal(vec2 uv, float t) {
    vec2 offset1 = rippleLayer(uv, t * 1.5, 15.0);
    vec2 offset2 = rippleLayer(uv + vec2(0.23, 0.47), t * 1.2 + 0.5, 22.0);
    
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

    // 2. 遮罩逻辑 (区分湿润区与深水区)
    float waterDepth = wetness - disp;
    float puddleMask = smoothstep(0.0, 0.1, waterDepth);
    float rippleMask = smoothstep(0.05, 0.1, waterDepth);

    // 3. PBR 属性混合
    vec3 dampAlbedo = albedo * 0.6;      
    vec3 puddleAlbedo = albedo * 0.2;    
    vec3 finalAlbedo = mix(dampAlbedo, puddleAlbedo, puddleMask);

    float dampRoughness = roughness * 0.8;
    float finalRoughness = mix(dampRoughness, 0.02, puddleMask);

    // 4. 法线混合 (解决波纹穿模)
    vec3 rippleNormal = getRainRippleNormal(TexCoords, time);
    vec3 flatWaterNormal = vec3(0.0, 0.0, 1.0);
    vec3 finalWaterNormal = normalize(mix(flatWaterNormal, rippleNormal, rippleMask));
    vec3 finalNormalMapValue = normalize(mix(normalMapValue, finalWaterNormal, puddleMask));

    // 5. TBN 转换
    vec3 T = vec3(1.0, 0.0, 0.0);
    vec3 B = vec3(0.0, 0.0, 1.0);
    vec3 N_geom = vec3(0.0, 1.0, 0.0);
    mat3 TBN = mat3(T, B, N_geom);
    vec3 N = normalize(TBN * finalNormalMapValue); 

    // --- [核心：找回原版质感] ---
    // 强制使用原版的高亮度和微蓝色调
    vec3 actualLightColor = vec3(0.8, 0.9, 1.0) * 4.5;
    
    // 强制把计算光照的光源高度拉低到 5.0，找回水面长倒影
    vec3 virtualLightPos = vec3(lightPos.x, 5.0, lightPos.z);

    // 6. 光照向量与衰减
    vec3 lightDir = normalize(virtualLightPos - FragPos);
    vec3 V = normalize(viewPos - FragPos);
    vec3 H = normalize(lightDir + V);

    float distToLight = length(virtualLightPos - FragPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distToLight + 0.032 * distToLight * distToLight);

    // 7. 计算光照分量 (使用 actualLightColor)
    float diff = max(dot(N, lightDir), 0.0);
    vec3 diffuse = diff * finalAlbedo * actualLightColor;

    float shininess = (1.0 - finalRoughness) * 200.0; 
    float spec = pow(max(dot(N, H), 0.0), shininess);
    float F0 = mix(0.04, 0.02, puddleMask); 
    float fresnel = F0 + (1.0 - F0) * pow(1.0 - max(dot(V, N), 0.0), 5.0);
    vec3 specular = actualLightColor * spec * (fresnel + (1.0 - finalRoughness) * 0.5) * mix(1.0, 5.0, puddleMask); 

    vec3 baseAmbient = vec3(0.01) * finalAlbedo * ao;
    vec3 fakeSkyReflect = vec3(0.02, 0.03, 0.05) * puddleMask; 
    vec3 ambient = baseAmbient + fakeSkyReflect;

    // 8. 统一混合所有光照
    vec3 color = ambient + (diffuse + specular) * attenuation;

    // 9. 绝对原汁原味的原版遮罩：2.5米内满亮，6.0米彻底死黑
    float distFromLightXZ = length(FragPos.xz - lightPos.xz);
    float spotlightMask = 1.0 - smoothstep(2.5, 6.0, distFromLightXZ);
    color *= spotlightMask;

    // 10. 色调映射与 Gamma 校正
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));

    // 11. 边缘无缝融入虚空背景 (修复正方形黑边)
    float distFromCenter = length(FragPos.xz);
    float voidFade = smoothstep(8.0, 10.0, distFromCenter);
    color = mix(color, vec3(0.05, 0.05, 0.05), voidFade);

    FragColor = vec4(color, 1.0);
}