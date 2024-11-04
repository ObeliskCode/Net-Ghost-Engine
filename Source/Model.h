#ifndef MODEL_H
#define MODEL_H

#ifndef NOASS
	#include <assimp/Importer.hpp>
	#include <assimp/scene.h>
	#include <assimp/postprocess.h>
#endif

#include "Mesh.h"
#include "Material.h"
#include <unordered_set>

class Model
{
public:
	Model(){}
	Model(const char* path)
	{
		loadModel(path);
	}
	~Model();
	void from_memory(std::string);
	void Draw(Shader& shader, Camera& camera,
		glm::mat4& transform, glm::mat4& ntransform);

	void DrawShadow(Shader& shader, glm::mat4& transform);

	//debug
	std::string m_name = " ";

	Material mat; // this mat here may have to stay
	// model data
	std::vector<Mesh> meshes;

private:

	void loadModel(std::string path);
#ifndef NOASS
	void processNode(aiNode* node, const aiScene* scene, aiMatrix4x4t<float> transformation, std::string directory, std::vector<Texture>& textures_loaded);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene, aiMatrix4x4t<float>& transformation, std::string directory, std::vector<Texture>& textures_loaded);
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, const aiScene* scene, aiTextureType type,
		std::string typeName, int slotInc, std::string directory, std::vector<Texture>& textures_loaded);
	glm::mat4 aiMat4toGLM(aiMatrix4x4t<float>& matrix);
#endif
};

#endif
