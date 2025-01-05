#pragma once
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

class Shader
{
private:
	unsigned int ID;

public:
	Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

	unsigned int getID();

	void useProgram();

	void setBool(const std::string& name, bool value) const;

	void setInt(const std::string& name, int value) const;

	void setFloat(const std::string& name, float value) const;

	void setMat3(const std::string& name, glm::mat3 value) const;

	void setMat4(const std::string& name, glm::mat4 value) const;

	void setVec3(const std::string& name, float x, float y, float z) const;

	void setVec3(const std::string& name, glm::vec3 value) const;
};

