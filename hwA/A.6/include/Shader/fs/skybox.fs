#version 330 core

in vec3 TexCoords; // �Ӷ�����ɫ������������
out vec4 FragColor; // ������ɫ

uniform sampler2D skybox_faces[5]; // ��� 2D ����

void main()
{
    vec2 tex_coords;
    int face_index;

    // ���� TexCoords �жϲ�����������
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

    // ����������ӳ�䵽 [0, 1] ��Χ
    tex_coords = tex_coords  + 0.5;

    // ʹ�ö�Ӧ�������
    FragColor = texture(skybox_faces[face_index], tex_coords);
}