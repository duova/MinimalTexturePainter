#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out vec3 FragPos;
out vec2 TexCoords;
out vec3 Normal;
out vec4 FragPosLightSpace;
out vec3 TangentLightDirection;
out vec3 TangentViewPos;
out vec3 TangentFragPos;

struct DirLight {
vec3 direction;
vec3 ambient;
vec3 diffuse;
vec3 specular;
};
uniform DirLight dirLight;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

uniform vec3 viewPos;

void main()
{
	FragPos = vec3(model * vec4(aPos, 1.0));
	TexCoords = aTexCoords;
	mat3 normalMatrix = transpose(inverse(mat3(model)));
	Normal = normalize(normalMatrix * aNormal);

	vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = Normal;
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));
    TangentLightDirection = TBN * dirLight.direction;
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;

	FragPosLightSpace = lightSpaceMatrix * vec4(FragPos, 1.0);
	gl_Position = projection * view * model * vec4(aPos, 1.0);
}