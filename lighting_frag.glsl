#version 330 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them 
// here for simplicity.

vec3 I = vec3(0.1, 0.1, 0.1);          // point light intensity
vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 kd = vec3(1, 0.2, 0.2);     // diffuse reflectance coefficient
vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
vec3 ks = vec3(0.8, 0.8, 0.8);   // specular reflectance coefficient

uniform vec3 viewPos;
uniform vec3 lightPositions[20];

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

in vec2 TexCoords;

out vec4 fragColor;

void main(void)
{
	// retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Albedo = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;

    kd = Albedo;
    vec3 ambientColor = Iamb * ka;
    vec3 lighting = vec3(0.0, 0.0, 0.0);

    for (int i = 0; i < 20; i++) {
	    vec3 L = normalize(lightPositions[i] - FragPos);
	    vec3 V = normalize(viewPos - FragPos);
	    vec3 H = normalize(L + V);
	    vec3 N = normalize(Normal);

	    float NdotL = dot(N, L); // for diffuse component
	    float NdotH = dot(N, H); // for specular component

	    vec3 diffuseColor = I * kd * max(0, NdotL);
	    vec3 specularColor = I * ks * pow(max(0, NdotH), 100);

        lighting += diffuseColor + specularColor;
    }

	fragColor = vec4(lighting, 1);
}