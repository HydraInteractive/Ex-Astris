#version 440 core

in vec2 texCoords;

layout(location = 0) out vec3 output;
layout(location = 1) uniform sampler2D positions;
layout(location = 2) uniform sampler2D diffuse;
layout(location = 3) uniform sampler2D normals;
layout(location = 4) uniform vec3 cameraPos;

void main() {
	vec3 pos = texture(positions, texCoords).rgb;
	vec3 color = texture(diffuse, texCoords).rgb;
	vec3 normal = texture(normals, texCoords).rgb;

	vec3 lightPos = vec3(0,0,-1);
	vec3 lightColor = vec3(1,1,1);

	vec3 lightDir = normalize(-lightPos);
	float diff = max(dot(normal, lightDir), 0.0f);

	//vec3 viewDir = normalize(cameraPos - pos);
	//vec3 reflectDir = reflect(-lightDir, normal);
	//float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
	//
	//vec3 d = lightColor * diff * color;
	//vec3 s = lightColor * spec;

	vec3 ambient = diffuse * 0.1f;

	output = (color * diff) + ambient;
}