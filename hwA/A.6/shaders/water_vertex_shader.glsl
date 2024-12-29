#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float time;

void main()
{
    vec2 waveOffset = vec2(sin(time), cos(time)) * 0.05;
    TexCoord = aTexCoord + waveOffset;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
