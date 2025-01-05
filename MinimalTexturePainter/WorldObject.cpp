#include "WorldObject.h"

WorldObject::WorldObject(const glm::mat4& transform, const std::shared_ptr<Model>& model)
{
    this->transform = transform;
    this->model = model;
}

void WorldObject::applyTransform(const glm::mat4& transform)
{
    this->transform = transform * this->transform;
}

glm::mat4 WorldObject::getTransform() const
{
    return transform;
}

void WorldObject::setTransform(const glm::mat4& transform)
{
    this->transform = transform;
}

const Model& WorldObject::getModel() const
{
    return *model;
}

void WorldObject::setModel(const std::shared_ptr<Model>& model)
{
    this->model = model;
}
