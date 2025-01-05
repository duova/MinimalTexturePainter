#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform sampler2D render;

void main()
{
	vec2 texCoords = (FragPos.xy + vec2(1, 1))/2;
	FragColor = texture(render, texCoords);
}