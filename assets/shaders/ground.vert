#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // 简单法线变换
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // [关键] 纹理坐标 * 10.0，让鹅卵石纹理在地面上重复平铺
    // 否则你会得到几块巨大的模糊石头
    TexCoords = aTexCoords; 
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}