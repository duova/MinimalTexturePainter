#pragma once
#include <vector>
#include <glm/glm.hpp>

#include "Shader.h"

struct DirectionalLight;
class WorldObject;

struct CameraParams
{
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 up;
	float fov; //vfov
	float aspect;
};

class Renderer
{
private:
	unsigned int shadowMapFramebuffer;
	unsigned int shadowMapTexture;
	unsigned int shadowWidth;
	unsigned int shadowHeight;
	Shader mainShader;
	Shader shadowShader;

public:
	Renderer(const Shader mainShader, const Shader shadowShader, const unsigned int shadowWidth, const unsigned int shadowHeight);

	void render(const unsigned int framebuffer, const CameraParams& cameraParams, const std::vector<WorldObject>& objects, const DirectionalLight& directionalLight, const int width, const int height);
};

