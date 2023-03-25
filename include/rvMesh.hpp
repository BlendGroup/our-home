#pragma once

#include <vector>
#include <string>
#include <GL/glew.h>
#include <assimp/types.h>

#include"vmath.h"
using namespace vmath;

#define MAX_BONE_INFLUENCE 4

// structure to store per vertex data to be passed to shader.
struct rvVertex
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	vec3 Tangent;
	vec3 Bitangent;
	ivec4 m_BoneID;
	vec4 m_Weight;

	rvVertex();
	void SetVertexBoneData(int boneID, float weight);
};

// structure to keep track of textures in model
struct rvTexture
{
	unsigned int id;
	std::string type;
	std::string path;
};

// structure to keep track of materials
struct rvMaterial
{
	std::string material_name;
	vec3 value;
	std::string type;
};

// strucute to keep track of each bone and it's offset as well as world transform
struct rvBoneMatrix
{
	aiMatrix4x4 aimat44_Offset_Matrix;//offset_matrix
	aiMatrix4x4 aimat44_Final_World_Transform;//final_world_transform
};

class rvMesh {
private:
	GLuint vao, vbo, ebo;
	std::vector<rvVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<rvTexture> textures;
	std::vector<rvMaterial> materials;
public:
	rvMesh(std::vector<rvVertex>& vertices, std::vector<unsigned int>& indices, std::vector<rvTexture>& textures, std::vector<rvMaterial>& mats);
	void MeshCleanup();
	void Draw(GLuint program);
	void DrawInstanced(GLuint program, GLuint numOfInstance);
	void printVertData();
	void setupMesh();
};