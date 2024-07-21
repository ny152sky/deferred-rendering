#version 330 core
out vec4 FragColor;
  
in vec2 TexCoord;

uniform sampler2D ourTexture;

// 'colorImage' is a sampler2D with the depth image
// read from the current depth buffer bound to it.
//
float LinearizeDepth(in vec2 uv)
{
    float zNear = 0.1;    // TODO: Replace by the zNear of your perspective projection
    float zFar  = 100.0; // TODO: Replace by the zFar  of your perspective projection
    float depth = texture(ourTexture, uv).x;
    return (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
    float c = LinearizeDepth(TexCoord);
    FragColor = vec4(c, c, c, 1.0);
}