#version 330 core

// All of the following variables could be defined in the OpenGL
// program and passed to this shader as uniform variables. This
// would be necessary if their values could change during runtim.
// However, we will not change them and therefore we define them 
// here for simplicity.

vec3 I = vec3(0.1, 0.1, 0.1);         // point light intensity
vec3 Iamb = vec3(0.8, 0.8, 0.8); // ambient light intensity
vec3 kd = vec3(1, 0.2, 0.2);     // diffuse reflectance coefficient
vec3 ka = vec3(0.3, 0.3, 0.3);   // ambient reflectance coefficient
vec3 ks = vec3(0.8, 0.8, 0.8);   // specular reflectance coefficient

uniform vec3 eyePos;
uniform vec3 objColor;
uniform vec3 lightPositions[20];

in vec4 fragWorldPos;
in vec3 fragWorldNor;

out vec4 fragColor;

vec3 lightLocations[40] = vec3[40](
    vec3(19.0, 2.0, 17.0), 
    vec3(17.0, 2.0, 15.0), 
    vec3(10.0, 2.0, 8.0),
    vec3(5.0, 2.0, 3.0), 
    vec3(0.0, 2.0, -2.0), 
    vec3(-5.0, 2.0, -7.0), 
    vec3(-10.0, 2.0, -17.0),
    vec3(-15.0, 2.0, -13.0), 
    vec3(-17.0, 2.0, -15.0), 
    vec3(-19.0, 2.0, -17.0),
    vec3(19.0, 2.0, 1.0), 
    vec3(17.0, 2.0, 3.0), 
    vec3(10.0, 2.0, 10.0),
    vec3(5.0, 2.0, 15.0), 
    vec3(0.0, 2.0, -20.0), 
    vec3(-5.0, 2.0, -15.0), 
    vec3(-10.0, 2.0, -10.0),
    vec3(-15.0, 2.0, -5.0), 
    vec3(-17.0, 2.0, -3.0), 
    vec3(-19.0, 2.0, -1.0),
    vec3(19.0, 3.0, 17.0), 
    vec3(17.0, 3.0, 15.0), 
    vec3(10.0, 3.0, 8.0),
    vec3(5.0, 3.0, 3.0), 
    vec3(0.0, 3.0, -2.0), 
    vec3(-5.0, 3.0, -7.0), 
    vec3(-10.0, 3.0, -17.0),
    vec3(-15.0, 3.0, -13.0), 
    vec3(-17.0, 3.0, -15.0), 
    vec3(-19.0, 3.0, -17.0),
    vec3(19.0, 3.0, 1.0), 
    vec3(17.0, 3.0, 3.0), 
    vec3(10.0, 3.0, 10.0),
    vec3(5.0, 3.0, 15.0), 
    vec3(0.0, 3.0, -20.0), 
    vec3(-5.0, 3.0, -15.0), 
    vec3(-10.0, 3.0, -10.0),
    vec3(-15.0, 3.0, -5.0), 
    vec3(-17.0, 3.0, -3.0), 
    vec3(-19.0, 3.0, -1.0)
);

void main(void)
{
	// Compute lighting. We assume lightPos and eyePos are in world
	// coordinates. fragWorldPos and fragWorldNor are the interpolated
	// coordinates by the rasterizer.

	kd = objColor;
	vec3 lighting = vec3(0.0, 0.0, 0.0);

	for (int i = 0; i < 20; i++) {
		vec3 L = normalize(lightPositions[i] - vec3(fragWorldPos));
		vec3 V = normalize(eyePos - vec3(fragWorldPos));
		vec3 H = normalize(L + V);
		vec3 N = normalize(fragWorldNor);

		float NdotL = dot(N, L); // for diffuse component
		float NdotH = dot(N, H); // for specular component

		vec3 diffuseColor = I * kd * max(0, NdotL);
		vec3 specularColor = I * ks * pow(max(0, NdotH), 100);

		lighting += diffuseColor + specularColor;
	}

	vec3 ambientColor = Iamb * ka;

	fragColor = vec4(lighting, 1);
}
