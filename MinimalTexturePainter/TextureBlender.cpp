#include "TextureBlender.h"

TextureBlender::TextureBlender(const Shader blendShader, unsigned int targetTexture, unsigned int sourceTexture):
	blendShader(blendShader)
{
	glGenFramebuffers(1, &targetFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
	glBindTexture(GL_TEXTURE_2D, targetTexture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, targetTexture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	this->targetTexture = targetTexture;
	this->sourceTexture = sourceTexture;

	//Gen buffers
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glGenBuffers(1, &quadEBO);

	//Buffer vertices
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVert), &quadVert, GL_STATIC_DRAW);

	//Buffer element data
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadElem), &quadElem, GL_STATIC_DRAW);

	//Set VAOs
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
}

void TextureBlender::blend(glm::vec2 uv, float pixelRadius, float alpha, float sourceScale, unsigned int texSize)
{
	glViewport(0, 0, texSize, texSize);
	glBindFramebuffer(GL_FRAMEBUFFER, targetFramebuffer);
	glEnable(GL_BLEND);
	//Don't clear existing.
	blendShader.useProgram();

	blendShader.setVec2("uv", uv);
	blendShader.setFloat("pixelRadius", pixelRadius);
	blendShader.setFloat("alpha", alpha);
	blendShader.setFloat("sourceScale", sourceScale);
	blendShader.setInt("texSize", (int)texSize);

	glActiveTexture(GL_TEXTURE0);
	glUniform1i(glGetUniformLocation(blendShader.getID(), "sourceTexture"), 0);
	glBindTexture(GL_TEXTURE_2D, sourceTexture);

	glBindVertexArray(quadVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_BLEND);
}

TextureBlender::~TextureBlender()
{
	glDeleteFramebuffers(1, &targetFramebuffer);
}
