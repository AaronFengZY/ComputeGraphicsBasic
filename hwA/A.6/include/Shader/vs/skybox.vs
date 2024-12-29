#version 330 core

layout (location = 0) in vec3 aPos; // ����λ��

uniform mat4 projection;  // ͶӰ����
uniform mat4 view;        // ��ͼ����
uniform mat4 model;      // ģ�;���

out vec3 TexCoords;       // ���ݸ�Ƭ����ɫ������������

void main() {
    // ����������ӳ�䵽 [-0.5, 0.5]��������������ͼ����
    TexCoords = aPos; 

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}