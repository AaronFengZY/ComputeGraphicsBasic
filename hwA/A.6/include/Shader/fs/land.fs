#version 330 core

in vec3 FragPos;   // �Ӷ�����ɫ��������Ƭ��λ�ã��������꣩
in vec2 TexCoords; // �Ӷ�����ɫ����������������

out vec4 FragColor; // �������ɫ

uniform sampler2D land_Texture;   // ���λ�������
uniform sampler2D detail_Texture; // ϸ������
uniform float detail_scale;       // ϸ�����������ϵ��

void main() {
    // �������λ�������
    vec3 baseColor = texture(land_Texture, TexCoords).rgb;

    // ����ϸ����������
    vec3 detailColor = texture(detail_Texture, TexCoords * detail_scale).rgb;

    // ��ϻ��������ϸ������
    float baseWeight = 0.8; // ���������Ȩ��
    float detailWeight = 1.0 - baseWeight; // ϸ�������Ȩ��
    vec3 finalColor = mix(baseColor, detailColor, detailWeight);

    FragColor = vec4(finalColor, 1.0);
}