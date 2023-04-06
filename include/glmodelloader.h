#ifndef __MODEL_LOADER__
#define __MODEL_LOADER__

#include"vmath.h"
#include<string>
#include<vector>
#include<unordered_map>

#define MAX_BONE_COUNT 100
#define MAX_BONE_INFLUENCE 4

struct Vertex {
	vmath::vec3 Position;
	vmath::vec3 Normal;
	vmath::vec2 TexCoords;
	vmath::vec3 Tangent;
	vmath::vec3 Bitangent;
	int BoneIDs[MAX_BONE_INFLUENCE];
	float Weights[MAX_BONE_INFLUENCE];
};

struct BoneInfoDL {
	int id;
	vmath::mat4 offset;
};

struct AssimpNodeDataDL {
	vmath::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeDataDL> children;
};

struct KeyPositionDL {
	vmath::vec3 position;
	float timeStamp;
};

struct KeyRotationDL {
	vmath::quaternion orientation;
	float timeStamp;
};

struct KeyScaleDL {
	vmath::vec3 scale;
	float timeStamp;
};

struct glmesh_dl {
	unsigned int vao;
	unsigned int trianglePointCount;
	std::vector<unsigned> diffuseTextures;
	std::vector<unsigned> specularTextures;
};

struct glbone_dl {
	int m_ID;
	std::string m_Name;
	std::vector<KeyPositionDL> m_Positions;
	std::vector<KeyRotationDL> m_Rotations;
	std::vector<KeyScaleDL> m_Scales;
	vmath::mat4 m_LocalTransform;
};

struct glanimator_dl {
	std::vector<vmath::mat4> m_FinalBoneMatrices;
	float m_Duration;
	int m_TicksPerSecond;
	std::vector<glbone_dl> m_Bones;
	AssimpNodeDataDL m_RootNode;
	std::unordered_map<std::string, BoneInfoDL> m_BoneInfoMap;
	float m_CurrentTime;
	float m_DeltaTime;
};

class glmodel {
private:
public:
	std::vector<glmesh_dl> meshes;
	std::vector<glanimator_dl> animator;
	std::unordered_map<std::string, BoneInfoDL> m_BoneInfoMap;
	int m_BoneCounter = 0;
	glmodel(std::string path, unsigned flags);
	void setBoneMatrixUniform(int i, int uniformLocation);
	std::string update(float delta, int i);
	void draw(int instance = 1);
};


#endif