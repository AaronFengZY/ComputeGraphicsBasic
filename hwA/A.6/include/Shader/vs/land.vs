#version 330 core

layout (location = 0) in vec3 aPos;       // 顶点位置
layout (location = 1) in vec2 aTexCoords; // 纹理坐标

uniform mat4 model;      // 模型矩阵
uniform mat4 view;       // 视图矩阵
uniform mat4 projection; // 投影矩阵

uniform sampler2D heightMap; // 高度图纹理
uniform float heightScale;   // 高度缩放系数

out vec3 FragPos;   // 传递到片段着色器的片段位置（世界坐标）
out vec2 TexCoords; // 传递到片段着色器的纹理坐标

uniform bool isUp;
uniform float offset;

void main() {
    // 将立方体底面上的 aPos.xz 转换为纹理坐标
    TexCoords = aPos.xz;


    // 调整顶点的 y 坐标，基础 y 坐标为 -0.5
    vec3 adjustedPos = vec3((aPos.x - 0.5)*0.2 , (aPos.y*heightScale + offset) , (aPos.z - 0.5)*0.2);

    // 检查 y 坐标，如果低于 -0.49，丢弃顶点
    if ((adjustedPos.y < -0.46 && isUp == true) || (isUp == false && adjustedPos.y < 0.46)) {
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0); // 丢弃顶点
        return;
    }

    // 将调整后的顶点坐标转换到世界坐标系
    FragPos = vec3(model * vec4(adjustedPos, 1.0));

    // 计算最终的剪辑坐标
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
