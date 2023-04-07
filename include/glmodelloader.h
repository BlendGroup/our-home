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

struct glmesh {
	GLuint vao;
	size_t trianglePointCount;
	GLuint diffuseTextures;
	GLuint specularTextures;
};

struct glbone {
	int id;
	std::string name;
	std::vector<KeyPosition> positions;
	std::vector<KeyRotation> rotations;
	std::vector<KeyScale> scales;
	vmath::mat4 localTransform;
};

struct glanimator {
	std::vector<vmath::mat4> finalBoneMatrices;
	float duration;
	int ticksPerSecond;
	std::vector<glbone> bones;
	AssimpNodeData rootNode;
	float currentTime;
};

class glmodel {
private:
public:
	std::vector<glmesh> meshes;
	std::vector<glanimator> animator;
	std::unordered_map<std::string, BoneInfo> boneInfoMap;
	int boneCounter = 0;
	glmodel(std::string path, unsigned flags);
	void setBoneMatrixUniform(GLuint uniformLocation, unsigned i);
	void update(float delta, int i);
	void draw(int instance = 1);
};


#endif