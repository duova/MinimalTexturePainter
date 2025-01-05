#pragma once
#include "Model.h"
#include "glm/glm.hpp"
#include <memory>

//This is used for all objects in the world space and contains the transform and model data.
class WorldObject
{
private:
	glm::mat4 transform;

	std::shared_ptr<Model> model;

public:
	WorldObject(const glm::mat4& transform, const std::shared_ptr<Model>& model);

	void applyTransform(const glm::mat4& transform);

	glm::mat4 getTransform() const;

	void setTransform(const glm::mat4& transform);

	const Model& getModel() const;

	void setModel(const std::shared_ptr<Model>& model);
};