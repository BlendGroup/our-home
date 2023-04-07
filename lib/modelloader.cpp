#include<string>
#include<fstream>
#include<sstream>
#include<iostream>
#include<unordered_map>
#include<vector>
#include<stack>
#include<GL/glew.h> 
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include"../include/gltextureloader.h"
#include"../include/vmath.h"
#include"../include/glmodelloader.h"
#include"../include/errorlog.h"

using namespace vmath;
using namespace std;

struct Vertex {
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
	vec3 Tangent;
	vec3 Bitangent;
	int BoneIDs[MAX_BONE_INFLUENCE];
	float Weights[MAX_BONE_INFLUENCE];
};

static vector<mat4> boneArrayDefault;

mat4 convertMatrixToVmathFormat(const aiMatrix4x4* from) {
	mat4 to;
	to[0][0] = from->a1; to[1][0] = from->a2; to[2][0] = from->a3; to[3][0] = from->a4;
	to[0][1] = from->b1; to[1][1] = from->b2; to[2][1] = from->b3; to[3][1] = from->b4;
	to[0][2] = from->c1; to[1][2] = from->c2; to[2][2] = from->c3; to[3][2] = from->c4;
	to[0][3] = from->d1; to[1][3] = from->d2; to[2][3] = from->d3; to[3][3] = from->d4;
	return to;
}

GLuint loadMaterialTextures(aiMaterial *mat, aiTextureType type, string directory) {
	if(mat->GetTextureCount(type) > 0) {
		aiString str;
		mat->GetTexture(type, 0, &str);
		return createTexture2D(directory + '/' + str.C_Str());
	} else {
		return 0;
	}
}

glbone* findBone(glanimator* a, string name) {
	for(int i = 0; i < a->bones.size(); i++) {
		if(a->bones[i].name == name) {
			return &a->bones[i];
		}
	}
	return NULL;
}

float getScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime) {
	float scaleFactor = 0.0f;
	float midWayLength = animationTime - lastTimeStamp;
	float framesDiff = nextTimeStamp - lastTimeStamp;
	scaleFactor = midWayLength / framesDiff;
	return scaleFactor;
}

void readHeirarchyData(AssimpNodeData& dest, const aiNode* src) {
	dest.name = src->mName.data;
	dest.transformation = convertMatrixToVmathFormat(&src->mTransformation);
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeData newData;
		readHeirarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}

void createAnimator(const aiScene* scene, glmodel *model) {
	for(int i = 0; i < scene->mNumAnimations; i++) {
		glanimator animator;
		aiAnimation* animation = scene->mAnimations[i];
		animator.currentTime = 0.0;
		for (int i = 0; i < 100; i++)
			animator.finalBoneMatrices.push_back(mat4::identity());

		animator.duration = animation->mDuration;
		animator.ticksPerSecond = animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		readHeirarchyData(animator.rootNode, scene->mRootNode);

		for (int i = 0; i < animation->mNumChannels; i++) {
			aiNodeAnim* channel = animation->mChannels[i];
			string boneName = channel->mNodeName.C_Str();

			if (model->boneInfoMap.count(boneName) == 0) {
				model->boneInfoMap[boneName].id = model->boneCounter;
				model->boneCounter++;
			}
			glbone b;
			b.name = channel->mNodeName.C_Str();
			b.id = model->boneInfoMap[channel->mNodeName.data].id;
			b.localTransform = mat4::identity();
			for (int positionIndex = 0; positionIndex < channel->mNumPositionKeys; ++positionIndex)
			{
				aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
				float timeStamp = channel->mPositionKeys[positionIndex].mTime;
				KeyPosition data;
				data.position = vec3(aiPosition.x, aiPosition.y, aiPosition.z);
				data.timeStamp = timeStamp;
				b.positions.push_back(data);
			}

			for (int rotationIndex = 0; rotationIndex < channel->mNumRotationKeys; ++rotationIndex)
			{
				aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
				float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
				KeyRotation data;
				data.orientation = quaternion(aiOrientation.w, vec3(aiOrientation.x, aiOrientation.y, aiOrientation.z));
				data.timeStamp = timeStamp;
				b.rotations.push_back(data);
			}

			for (int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex)
			{
				aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
				float timeStamp = channel->mScalingKeys[keyIndex].mTime;
				KeyScale data;
				data.scale = vec3(scale.x, scale.y, scale.z);
				data.timeStamp = timeStamp;
				b.scales.push_back(data);
			}
			animator.bones.push_back(b);
		}
		model->animator.push_back(animator);
	}
}

void createMesh(const aiScene* scene, glmodel *model, string directory) {
	//Process Node Tree
	stack<aiNode*> nodeTreeStack;
	nodeTreeStack.push(scene->mRootNode);
	while(!nodeTreeStack.empty()) {
		//Select Current Node
		aiNode* node = nodeTreeStack.top();
		nodeTreeStack.pop();
		//Process All Meshes for Current Node
		for(unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			glmesh m;
			vector<Vertex> vertices;
			vector<unsigned int> indices;

			//Fill Vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;
				if(mesh->HasPositions()) {
					vertex.Position = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
				} else {
					vertex.Position = vec3(0.0f, 0.0f, 0.0f);
				}
				if(mesh->HasNormals()) {
					vertex.Normal = vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
				} else {
					vertex.Normal = vec3(0.0f);
				}
				if (mesh->HasTextureCoords(0)) {
					vertex.TexCoords = vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
				} else {
					vertex.TexCoords = vec2(0.0f, 0.0f);
				}
				if(mesh->HasTangentsAndBitangents()) {
					vertex.Tangent = vec3(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
					vertex.Bitangent = vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z);
				} else {
					vertex.Tangent = vec3(0.0f);
					vertex.Bitangent = vec3(0.0f);
				}
				for(int i = 0; i < MAX_BONE_INFLUENCE; i++) {
					vertex.BoneIDs[i] = -1;
					vertex.Weights[i] = 0.0f;
				}
				vertices.push_back(vertex);
			}

			//Fill Indices
			for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
				aiFace face = mesh->mFaces[i];
				for (unsigned int j = 0; j < face.mNumIndices; j++) {
					indices.push_back(face.mIndices[j]);
				}
			}

			//Fill Textures
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
			m.diffuseTextures = loadMaterialTextures(material, aiTextureType_DIFFUSE, directory);
			m.specularTextures = loadMaterialTextures(material, aiTextureType_SPECULAR, directory);

			// Fill Bones and Weights
			for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
				int boneID = -1;
				string boneName = mesh->mBones[boneIndex]->mName.C_Str();
				if (model->boneInfoMap.count(boneName) == 0) {
					BoneInfo newBoneInfo;
					newBoneInfo.id = model->boneCounter;
					newBoneInfo.offset = convertMatrixToVmathFormat(&mesh->mBones[boneIndex]->mOffsetMatrix);
					model->boneInfoMap[boneName] = newBoneInfo;
					boneID = model->boneCounter;
					model->boneCounter++;
				} else {
					boneID = model->boneInfoMap[boneName].id;
				}
				aiVertexWeight* weights = mesh->mBones[boneIndex]->mWeights;
				int numWeights = mesh->mBones[boneIndex]->mNumWeights;

				for (int weightIndex = 0; weightIndex < numWeights; ++weightIndex) {
					int vertexId = weights[weightIndex].mVertexId;
					float weight = weights[weightIndex].mWeight;
					for (int i = 0; i < MAX_BONE_INFLUENCE; ++i) {
						if (vertices[vertexId].BoneIDs[i] < 0) {
							vertices[vertexId].Weights[i] = weight;
							vertices[vertexId].BoneIDs[i] = boneID;
							break;
						}
					}
				}
			}

			GLuint VBO, EBO;
			glGenVertexArrays(1, &m.vao);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(m.vao);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
			glEnableVertexAttribArray(5);
			glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, BoneIDs));
			glEnableVertexAttribArray(6);
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Weights));
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

			m.trianglePointCount = indices.size();

			model->meshes.push_back(m);
		}
		//Add Childern To NodeList
		for(unsigned int i = 0; i < node->mNumChildren; i++) {
			nodeTreeStack.push(node->mChildren[i]);
		}
	}
}

glmodel::glmodel(string path, unsigned flags) {
	//Open File
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, flags);
	if(!scene) {
		throwErr("Error: Assimp could not load scene for '" + path + "'");
	} else if(scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
		throwErr("Error: Assimp scene incomplete for '" + path + "'");
	} else if(!scene->mRootNode) {
		throwErr("Error: Assimp root node empty for '" + path + "'");
	}
	if(scene->HasMeshes()) {
		createMesh(scene, this, path.substr(0, path.find_last_of('/')));
	}

	if(scene->HasAnimations()) {
		createAnimator(scene, this);
	}

	importer.FreeScene();
}

void calculateBoneTransform(glmodel* model, glanimator* a, const AssimpNodeData* node, mat4 parentTransform) {
	string nodeName = node->name;
	mat4 nodeTransform = node->transformation;

	glbone* bone = findBone(a, nodeName);

	if(bone) {
		mat4 translationMat;
		mat4 rotationMat;
		mat4 scalingMat;
		int p0Index = -1;
		int p1Index = -1;

		//Calculate Translation
		if (bone->positions.size() == 1) {
			translationMat = translate(bone->positions[0].position);
		} else {
			for(p0Index = 0; p0Index < bone->positions.size() - 1; ++p0Index) {
				if (a->currentTime < bone->positions[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == bone->positions.size() - 1) {
				translationMat = translate(bone->positions[p0Index].position);
			} else {
				p1Index = p0Index + 1;
				translationMat = translate(mix(bone->positions[p0Index].position, bone->positions[p1Index].position, getScaleFactor(bone->positions[p0Index].timeStamp, bone->positions[p1Index].timeStamp, a->currentTime)));
			}
		}

		//Calculate Rotation
		if (bone->rotations.size() == 1) {
			rotationMat = quaternionToMatrix(normalize(bone->rotations[0].orientation));
		} else {
			for(p0Index = 0; p0Index < bone->rotations.size() - 1; ++p0Index) {
				if (a->currentTime < bone->rotations[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == bone->rotations.size() - 1) {
				rotationMat = quaternionToMatrix(normalize(bone->rotations[p0Index].orientation));
			} else {
				p1Index = p0Index + 1;
				rotationMat = quaternionToMatrix(normalize(slerp(bone->rotations[p0Index].orientation, bone->rotations[p1Index].orientation, getScaleFactor(bone->rotations[p0Index].timeStamp, bone->rotations[p1Index].timeStamp, a->currentTime))));
			}
		}

		//Calculate Scale
		if (bone->scales.size() == 1) {
			scalingMat = scale(bone->scales[0].scale);
		} else {
			for(p0Index = 0; p0Index < bone->scales.size() - 1; ++p0Index) {
				if (a->currentTime < bone->scales[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == bone->scales.size() - 1) {
				scalingMat = scale(bone->scales[p0Index].scale);
			} else {
				p1Index = p0Index + 1;
				scalingMat = scale(mix(bone->scales[p0Index].scale, bone->scales[p1Index].scale, getScaleFactor(bone->scales[p0Index].timeStamp, bone->scales[p1Index].timeStamp, a->currentTime)));
			}
		}

		bone->localTransform = translationMat * rotationMat * scalingMat;
		nodeTransform = bone->localTransform;
	}

	mat4 globalTransformation = parentTransform * nodeTransform;

	unordered_map<string, BoneInfo> boneInfoMap = model->boneInfoMap;
	if (boneInfoMap.count(nodeName) != 0)
	{
		int index = boneInfoMap[nodeName].id;
		mat4 offset = boneInfoMap[nodeName].offset;
		a->finalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++) {
		calculateBoneTransform(model, a, &node->children[i], globalTransformation);
	}
}

void glmodel::setBoneMatrixUniform(int i, int uniformLocation) {
	vector<mat4> m;
	if(i < this->animator.size() && i >= 0) {
		m = this->animator[i].finalBoneMatrices;
	} else {
		if(boneArrayDefault.empty()) {
			for(int i = 0; i < MAX_BONE_COUNT; i++) {
				boneArrayDefault.push_back(mat4::identity());
			}
		}
		m = boneArrayDefault;
	}
	glUniformMatrix4fv(uniformLocation, MAX_BONE_COUNT, GL_FALSE, *m.data());
}

void glmodel::update(float dt, int i) {
	if(i < this->animator.size()) {
		this->animator[i].currentTime += this->animator[i].ticksPerSecond * dt;
		this->animator[i].currentTime = fmod(this->animator[i].currentTime, this->animator[i].duration);
		calculateBoneTransform(this, &this->animator[i], &this->animator[i].rootNode, mat4::identity());
	}
}

void glmodel::draw(int instance) {
	for(unsigned int i = 0; i < this->meshes.size(); i++) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, this->meshes[i].diffuseTextures);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, this->meshes[i].specularTextures);
		glBindVertexArray(this->meshes[i].vao);
		glDrawElementsInstanced(GL_TRIANGLES, this->meshes[i].trianglePointCount, GL_UNSIGNED_INT, 0, instance);
	}
}
