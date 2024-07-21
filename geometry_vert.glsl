#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 FragPos;
out vec3 Normal;
out vec3 Color;
out float SpecularIntensity;
out vec3 EmissiveColor;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;
uniform vec3 objColor;
uniform float objSpec;
uniform vec3 objEmissiveColor;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0f);
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = normalize(mat3(inverse(transpose(model))) * aNormal);
    Color = objColor;
    SpecularIntensity = objSpec;
    EmissiveColor = objEmissiveColor;
}
