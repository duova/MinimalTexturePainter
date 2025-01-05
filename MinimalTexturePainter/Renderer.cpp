#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "DirectionalLight.h"
#include "Shader.h"
#include "WorldObject.h"

Renderer::Renderer(const Shader mainShader, const Shader shadowShader, const unsigned int shadowWidth, const unsigned int shadowHeight):
	mainShader(mainShader),
	shadowShader(shadowShader)
{
	this->shadowWidth = shadowWidth;
	this->shadowHeight = shadowHeight;
	glGenFramebuffers(1, &shadowMapFramebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);
	glGenTextures(1, &shadowMapTexture);
	glBindTexture(GL_TEXTURE_2D, shadowMapTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowWidth, shadowHeight, 0, GL_DEPTH_COMPONENT,
	             GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMapTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::render(const unsigned int framebuffer, const CameraParams& cameraParams, const std::vector<WorldObject>& objects, const DirectionalLight& directionalLight, const int width, const int height)
{
	//Shadow mapping pass.
	glViewport(0, 0, shadowWidth, shadowHeight);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFramebuffer);
	glClear(GL_DEPTH_BUFFER_BIT);
	shadowShader.useProgram();

	constexpr float nearPlane = 1.0f;
	constexpr float farPlane = 20.0f;
	glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
	glm::mat4 lightView = glm::lookAt(-glm::normalize(cameraParams.forward) * 10.f,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 lightSpace = lightProjection * lightView;
	shadowShader.setMat4("lightSpace", lightSpace);

	for (const WorldObject& object : objects)
	{
		glm::mat4 model = object.getTransform();

		shadowShader.setMat4("model", model);

		object.getModel().draw(shadowShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Main pass.
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mainShader.useProgram();

	glm::mat4 view = glm::lookAt(cameraParams.position, cameraParams.position + cameraParams.forward, cameraParams.up);
	glm::mat4 projection = glm::perspective(cameraParams.fov, cameraParams.aspect, 0.1f, 100.0f);

	mainShader.setFloat("material.shininess", 32.0f);
	mainShader.setVec3("viewPos", cameraParams.position);
	mainShader.setMat4("view", view);
	mainShader.setMat4("projection", projection);
	mainShader.setVec3("dirLight.direction", directionalLight.direction);
	mainShader.setVec3("dirLight.ambient", directionalLight.ambient);
	mainShader.setVec3("dirLight.diffuse", directionalLight.diffuse);
	mainShader.setVec3("dirLight.specular", directionalLight.specular);

	for (const WorldObject& object : objects)
	{
		glm::mat4 model = object.getTransform();
		glm::mat3 normalMatrix = transpose(inverse(model));

		mainShader.setMat4("model", model);
		mainShader.setMat3("normalMatrix", normalMatrix);

		object.getModel().draw(mainShader);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
