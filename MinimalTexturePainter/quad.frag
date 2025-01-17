#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform sampler2D render;

void main()
{
	vec2 texCoords = (FragPos.xy + vec2(1, 1))/2;
	FragColor = texture(render, texCoords);

	//Apply gamma correction.
    float gamma = 2.2;
    FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
}
