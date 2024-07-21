#version 330 core

uniform sampler2D gAlbedoSpec;
uniform sampler2D gTexDepth;

in vec2 TexCoords;

out vec4 fragColor;

float pivot = 0.0;

vec4 blur(float factor, float depth, vec2 texSize)
{
    int blurSize = int(abs(depth - pivot) * factor);
    vec4 sum = vec4(0, 0, 0, 0);
    for (int y = -blurSize; y <= blurSize; ++y)
    {
        for (int x = -blurSize; x <= blurSize; ++x)
        {
            sum += texture(gAlbedoSpec, (TexCoords.xy + vec2(x, y)) / texSize);
        }
    }
    return sum / ((blurSize*2 + 1) * (blurSize*2 + 1));
}

void main(void)
{
    vec2 texSize = textureSize(gAlbedoSpec, 0);
    float d = texture(gTexDepth, TexCoords.xy / texSize).r;
    fragColor = blur(5, d, texSize);
}