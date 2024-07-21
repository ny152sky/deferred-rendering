#version 330 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 gEmissive;

in vec3 FragPos;
in vec3 Normal;
in vec3 Color;
in float SpecularIntensity;
in vec3 EmissiveColor;

void main()
{    
    // store the fragment position vector in the first gbuffer texture
    gPosition = FragPos;
    gNormal = normalize(Normal);
    // and the diffuse per-fragment color
    gAlbedoSpec.rgb = Color;
    // store specular intensity in gAlbedoSpec's alpha component
    gAlbedoSpec.a = SpecularIntensity;

    gEmissive = EmissiveColor;
}