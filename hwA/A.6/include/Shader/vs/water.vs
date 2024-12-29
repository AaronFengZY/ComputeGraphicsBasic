#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec2 TexCoords; // ���ݵ�Ƭ����ɫ������������
out vec3 Normal;    // ���ݵ�Ƭ����ɫ���ķ�����
out vec3 FragPos;   // ���ݵ�Ƭ����ɫ����Ƭ��λ��

void main()
{
    
    // ��Ƭ��λ�ü���Ϊ��������
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // �任������
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    TexCoords = aPos.xz; // ���� aPos.xz �Ѿ��� [-0.5, 0.5]
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
