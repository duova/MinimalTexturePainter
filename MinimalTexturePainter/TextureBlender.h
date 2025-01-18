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

public:
	TextureBlender(const Shader blendShader, unsigned int targetTexture, unsigned int sourceTexture);

	void blend(glm::vec2 uv, float pixelRadius, float alpha, float sourceScale);
};

