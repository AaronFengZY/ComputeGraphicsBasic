#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords; // 传递到片段着色器的纹理坐标
out vec3 Normal;    // 传递到片段着色器的法向量
out vec3 FragPos;   // 传递到片段着色器的片段位置

void main()
{
    
    // 将片段位置计算为世界坐标
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // 变换法向量
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    TexCoords = aPos.xz; // 假设 aPos.xz 已经在 [-0.5, 0.5]
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
