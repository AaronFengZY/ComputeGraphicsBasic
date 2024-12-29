#version 330 core

in vec2 TexCoords; // 从顶点着色器传来的纹理坐标
in vec3 Normal;    // 从顶点着色器传来的法向量
in vec3 FragPos;   // 从顶点着色器传来的片段位置

out vec4 FragColor; // 输出的颜色

uniform sampler2D texture; // 水面纹理
uniform float water_alpha; // 水面透明度
uniform float xShift;      // 动态波纹X方向偏移
uniform float yShift;      // 动态波纹Y方向偏移
uniform float texture_scale; // 控制纹理的重复次数

void main()
{
    // 将 [-0.5, 0.5] 的纹理坐标映射到 [0, 1] 范围
    vec2 scaledTexCoords = TexCoords * 0.5 + 0.5;

    // 增加纹理重复，通过 texture_scale 放大纹理采样范围
    vec2 dynamicTexCoords = scaledTexCoords * texture_scale + vec2(xShift, yShift);

    // 采样水面纹理
    vec4 baseColor = texture(texture, dynamicTexCoords);

    // 简单的反射效果 (法向量与视角方向的结合)
    vec3 viewDir = normalize(-FragPos); // 从片段到相机的方向
    float reflectStrength = dot(normalize(Normal), viewDir);

    // 混合水面颜色和反射强度
    vec3 finalColor = mix(baseColor.rgb, vec3(0.8, 0.9, 1.0), clamp(reflectStrength, 0.0, 1.0));

    FragColor = vec4(finalColor, water_alpha); // 应用透明度
}
