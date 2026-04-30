#version 330 core

layout(location = 0) in vec3 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

uniform mat4 modelViewProj;

void main()
{
    gl_Position = modelViewProj * vec4(inPos, 1.0);
}