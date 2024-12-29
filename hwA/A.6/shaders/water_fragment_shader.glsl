#version 330 core
out vec4 FragColor;
in vec2 TexCoord;

uniform sampler2D waterTexture;

void main()
{
    FragColor = texture(waterTexture, TexCoord);
}
