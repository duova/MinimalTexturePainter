#pragma once
#include "Shader.h"
#include "Mesh.h"
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model(const char* path)
	{
		loadModel(path);
	}
	void draw(Shader& shader) const;

	vector<Mesh> meshes;
	vector<Texture> textures_loaded;

private:
	string directory;

	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadMaterialTextures(aiMaterial* mat,
		aiTextureType type, string typeName);
	unsigned int textureFromFile(const char* path, const string& directory, bool useSRGB);
};