#version 330 core

in vec2 TexCoords; // �Ӷ�����ɫ����������������
in vec3 Normal;    // �Ӷ�����ɫ�������ķ�����
in vec3 FragPos;   // �Ӷ�����ɫ��������Ƭ��λ��

out vec4 FragColor; // �������ɫ

uniform sampler2D texture; // ˮ������
uniform float water_alpha; // ˮ��͸����
uniform float xShift;      // ��̬����X����ƫ��
uniform float yShift;      // ��̬����Y����ƫ��
uniform float texture_scale; // ����������ظ�����

void main()
{
    // �� [-0.5, 0.5] ����������ӳ�䵽 [0, 1] ��Χ
    vec2 scaledTexCoords = TexCoords * 0.5 + 0.5;

    // ���������ظ���ͨ�� texture_scale �Ŵ����������Χ
    vec2 dynamicTexCoords = scaledTexCoords * texture_scale + vec2(xShift, yShift);

    // ����ˮ������
    vec4 baseColor = texture(texture, dynamicTexCoords);

    // �򵥵ķ���Ч�� (���������ӽǷ���Ľ��)
    vec3 viewDir = normalize(-FragPos); // ��Ƭ�ε�����ķ���
    float reflectStrength = dot(normalize(Normal), viewDir);

    // ���ˮ����ɫ�ͷ���ǿ��
    vec3 finalColor = mix(baseColor.rgb, vec3(0.8, 0.9, 1.0), clamp(reflectStrength, 0.0, 1.0));

    FragColor = vec4(finalColor, water_alpha); // Ӧ��͸����
}
