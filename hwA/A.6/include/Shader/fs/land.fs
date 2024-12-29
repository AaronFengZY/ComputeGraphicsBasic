#version 330 core

in vec3 FragPos;   // 从顶点着色器传来的片段位置（世界坐标）
in vec2 TexCoords; // 从顶点着色器传来的纹理坐标

out vec4 FragColor; // 输出的颜色

uniform sampler2D land_Texture;   // 地形基础纹理
uniform sampler2D detail_Texture; // 细节纹理
uniform float detail_scale;       // 细节纹理的缩放系数

void main() {
    // 采样地形基础纹理
    vec3 baseColor = texture(land_Texture, TexCoords).rgb;

    // 采样细节纹理并缩放
    vec3 detailColor = texture(detail_Texture, TexCoords * detail_scale).rgb;

    // 混合基础纹理和细节纹理
    float baseWeight = 0.8; // 基础纹理的权重
    float detailWeight = 1.0 - baseWeight; // 细节纹理的权重
    vec3 finalColor = mix(baseColor, detailColor, detailWeight);

    FragColor = vec4(finalColor, 1.0);
}