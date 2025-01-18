#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Shader.h"

class TextureBlender
{
private:
	unsigned int targetFramebuffer;
	unsigned int targetTexture;
	unsigned int sourceTexture;
	Shader blendShader;

	float quadVert[12] = { 1.f, 1.f, 0,
					1.f, -1.f, 0,
					-1.f, -1.f, 0,
					-1.f, 1.f, 0 };

	unsigned int quadElem[6] = { 0, 1, 3,
						1, 2, 3 };

	unsigned int quadVBO;
	unsigned int quadVAO;
	unsigned int quadEBO;

public:
	TextureBlender(const Shader blendShader, unsigned int targetTexture, unsigned int sourceTexture);

	void blend(glm::vec2 uv, float pixelRadius, float alpha, float sourceScale, unsigned int texSize);

	~TextureBlender();
};

