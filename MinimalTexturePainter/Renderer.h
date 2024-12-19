#pragma once
#include <vector>

class WorldObject;

class Renderer
{
private:


public:
	void AddObjects(const std::vector<WorldObject>& objects);

	void Draw();
};

