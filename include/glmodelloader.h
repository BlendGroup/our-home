#ifndef __MODEL_LOADER__
#define __MODEL_LOADER__

#include"vmath.h"
#include<string>
#include<vector>
#include<unordered_map>

#define MAX_BONE_COUNT 100
#define MAX_BONE_INFLUENCE 4

struct BoneInfo {
	int id;
	vmath::mat4 offset;
};

struct AssimpNodeData {
	vmath::mat4 transformation;
	std::string name;
	int childrenCount;
	std::vector<AssimpNodeData> children;
};

struct KeyPosition {
	vmath::vec3 position;
	float timeStamp;
};

struct KeyRotation {
	vmath::quaternion orientation;
	float timeStamp;
};

struct KeyScale {
	vmath::vec3 scale;
	float timeStamp;
};

struct glmesh_dl {
	GLuint vao;
	size_t trianglePointCount;
	GLuint diffuseTextures;
	GLuint specularTextures;
};

struct glbone_dl {
	int id;
	std::string name;
	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;
	std::vector<KeyScale> scales;
	vmath::mat4 localTransform;
};

struct glanimator_dl {
	std::vector<vmath::mat4> finalBoneMatrices;
	float duration;
	int ticksPerSecond;
	std::vector<glbone_dl> bones;
	AssimpNodeData rootNode;
	std::unordered_map<std::string, BoneInfo> boneInfoMap;
	float currentTime;
};

class glmodel {
private:
public:
	std::vector<glmesh_dl> meshes;
	std::vector<glanimator_dl> animator;
	std::unordered_map<std::string, BoneInfo> boneInfoMap;
	int boneCounter = 0;
	glmodel(std::string path, unsigned flags);
	void setBoneMatrixUniform(int uniformLocation, int i);
	void update(float delta, int i);
	void draw(int instance = 1);
};


#endif