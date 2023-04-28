#ifndef __MODEL_LOADER__
#define __MODEL_LOADER__

#include<vmath.h>
#include<string>
#include<vector>
#include<unordered_map>
#include<GL/glew.h>
#include<GL/gl.h>
#include<glshaderloader.h>

#define MAX_BONE_COUNT 100
#define MAX_BONE_INFLUENCE 4

enum textureTypes {
	TEX_DIFFUSE = 0,
	TEX_SPECULAR = 1,
	TEX_NORMAL = 2,
	TEX_AO = 3,
	TEX_ROUGHNESS = 4,
	TEX_METALIC = 5,
	TEX_GLOSSINESS = 6,
	TEX_EMISSIVE = 7
};

enum materialTypes {
	MAT_AMBIENT = 0,
	MAT_DIFFUSE = 1,
	MAT_SPECULAR = 2,
	MAT_EMISSIVE = 3,
	MAT_SHININESS = 4,
	MAT_OPACITY = 5
};

// structure to keep track of textures in model
struct texture {
	GLuint id;
	textureTypes type;
};

// structure to keep track of materials
struct material {
	vmath::vec3 value;
	materialTypes type;
};

struct glmaterial{
	vmath::vec3 ambient;
	vmath::vec3 diffuse;
	vmath::vec3 specular;
	vmath::vec3 emissive; // may or may not use
	float shininess;
	float opacity;
	std::vector<texture> textures;
};

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
	GLint materialIndex;
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
	std::vector<glmaterial> materials;
	int boneCounter = 0;
	glmodel(std::string path, unsigned flags, bool isPbr);
	void setBoneMatrixUniform(GLuint uniformLocation, unsigned i);
	void setBoneMatrixUniform(GLuint uniformLocation, GLuint bMat,unsigned i);
	void update(float dt, int baseAnimation , int layeredAnimation = 0, float blendFactor = 0.0f );
	void draw(glshaderprogram* program,int instance = 1);
};


#endif