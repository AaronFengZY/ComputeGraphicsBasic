#version 330 core

layout (location = 0) in vec3 aPos;       // ����λ��
layout (location = 1) in vec2 aTexCoords; // ��������

uniform mat4 model;      // ģ�;���
uniform mat4 view;       // ��ͼ����
uniform mat4 projection; // ͶӰ����

uniform sampler2D heightMap; // �߶�ͼ����
uniform float heightScale;   // �߶�����ϵ��

out vec3 FragPos;   // ���ݵ�Ƭ����ɫ����Ƭ��λ�ã��������꣩
out vec2 TexCoords; // ���ݵ�Ƭ����ɫ������������

uniform bool isUp;
uniform float offset;

void main() {
    // ������������ϵ� aPos.xz ת��Ϊ��������
    TexCoords = aPos.xz;


    // ��������� y ���꣬���� y ����Ϊ -0.5
    vec3 adjustedPos = vec3((aPos.x - 0.5)*0.2 , (aPos.y*heightScale + offset) , (aPos.z - 0.5)*0.2);

    // ��� y ���꣬������� -0.49����������
    if ((adjustedPos.y < -0.46 && isUp == true) || (isUp == false && adjustedPos.y < 0.46)) {
        gl_Position = vec4(0.0, 0.0, 0.0, 0.0); // ��������
        return;
    }

    // ��������Ķ�������ת������������ϵ
    FragPos = vec3(model * vec4(adjustedPos, 1.0));

    // �������յļ�������
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
