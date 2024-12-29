#version 330 core

in vec3 TexCoords; // 从顶点着色器传来的坐标
out vec4 FragColor; // 最终颜色

uniform sampler2D skybox_faces[5]; // 五个 2D 纹理

void main()
{
    vec2 tex_coords;
    int face_index;

    // 根据 TexCoords 判断采样的纹理面
    if (abs(TexCoords.x) > abs(TexCoords.y) && abs(TexCoords.x) > abs(TexCoords.z)) {
        if (TexCoords.x > 0.0) { // Right (+X)
            face_index = 1;
            tex_coords = vec2(TexCoords.z, TexCoords.y);
        } else { // Left (-X)
            face_index = 3;
            tex_coords = vec2(-TexCoords.z, TexCoords.y);
        }
    } else if (abs(TexCoords.y) > abs(TexCoords.x) && abs(TexCoords.y) > abs(TexCoords.z)) {
        if (TexCoords.y > 0.0) { // Top (+Y)
            face_index = 4;
            tex_coords = vec2(TexCoords.x, TexCoords.z);
        } 
    } else {
        if (TexCoords.z > 0.0) { // Front (+Z)
            face_index = 2;
            tex_coords = vec2(-TexCoords.x, TexCoords.y);
        } else { // Back (-Z)
            face_index = 0;
            tex_coords = vec2(TexCoords.x, TexCoords.y);
        }
    }

    // 将纹理坐标映射到 [0, 1] 范围
    tex_coords = tex_coords  + 0.5;

    // 使用对应的面采样
    FragColor = texture(skybox_faces[face_index], tex_coords);
}