#version 330 core

layout (location = 0) in vec3 aPos; // 顶点位置

uniform mat4 projection;  // 投影矩阵
uniform mat4 view;        // 视图矩阵
uniform mat4 model;      // 模型矩阵

out vec3 TexCoords;       // 传递给片段着色器的纹理坐标

void main() {
    // 将顶点坐标映射到 [-0.5, 0.5]，用于立方体贴图采样
    TexCoords = aPos; 

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}