#version 420 core

in vec3 fragPos;
in vec3 fragNor;
in vec2 fragTexCoord;

out vec4 color;

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

layout (std140, binding = 0) uniform CameraData
{
	mat4 view;
	mat4 projection;
	vec4 cameraPos;
};

layout(std140, binding = 1) uniform LightData
{
	vec4 lightDir;
};

uniform Material mat;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 view = normalize(cameraPos.xyz - fragPos);
	vec3 light = normalize(-lightDir.xyz);
	
	// Ambient light
	vec3 ambient = mat.ambient;

	// Diffuse reflection
	float dC = max(dot(normal, light), 0.0);
	vec3 diffuse = mat.diffuse * dC;

	// Specular reflection
	vec3 halfDir = normalize(light + view);
	float sC = max(dot(normal, halfDir), 0.0);
	vec3 specular = mat.specular * pow(sC, mat.shininess);

	vec3 reflection = ambient + diffuse + specular;
	color = vec4(reflection, 1.0);
}