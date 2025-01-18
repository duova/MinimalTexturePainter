#version 330 core
out vec4 FragColor;

in vec3 FragPos;

uniform vec2 uv;
uniform float pixelRadius;
uniform float alpha;
uniform float sourceScale;
uniform int texSize;
uniform sampler2D sourceTexture;

void main()
{
	vec2 texCoords = (FragPos.xy + vec2(1, 1))/2;

	float uvRadius = pixelRadius / float(texSize);
	float distance = length(texCoords - uv);
	float multiplier = 1.0 - (min(floor(distance / uvRadius), 1.0)); //1 when inside radius 0 when outside radius.

	if (multiplier < 0.99) discard;

	FragColor = texture(sourceTexture, texCoords / sourceScale) * multiplier;
	FragColor = vec4(FragColor.rgb, FragColor.a * alpha);
}
