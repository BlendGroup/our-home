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

static vector<mat4> boneArrayDefault;

mat4 convertMatrixToVmathFormat(const aiMatrix4x4* from) {
	mat4 to;
	to[0][0] = from->a1; to[1][0] = from->a2; to[2][0] = from->a3; to[3][0] = from->a4;
	to[0][1] = from->b1; to[1][1] = from->b2; to[2][1] = from->b3; to[3][1] = from->b4;
	to[0][2] = from->c1; to[1][2] = from->c2; to[2][2] = from->c3; to[3][2] = from->c4;
	to[0][3] = from->d1; to[1][3] = from->d2; to[2][3] = from->d3; to[3][3] = from->d4;
	return to;
}

string loadMaterialTextures(aiMaterial *mat, aiTextureType type, string directory, vector<unsigned> &textureVector) {
	stringstream errStr;
	for(unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
		aiString str;
		mat->GetTexture(type, i, &str);
		textureVector.push_back(createTexture2D(directory + '/' + str.C_Str()));
	}
	return errStr.str();
}

glbone_dl* findBone(glanimator_dl* a, string name) {
	for(int i = 0; i < a->m_Bones.size(); i++) {
		if(a->m_Bones[i].m_Name == name) {
			return &a->m_Bones[i];
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

void readHeirarchyData(AssimpNodeDataDL& dest, const aiNode* src) {
	dest.name = src->mName.data;
	dest.transformation = convertMatrixToVmathFormat(&src->mTransformation);
	dest.childrenCount = src->mNumChildren;

	for (int i = 0; i < src->mNumChildren; i++)
	{
		AssimpNodeDataDL newData;
		readHeirarchyData(newData, src->mChildren[i]);
		dest.children.push_back(newData);
	}
}

string createAnimator(const aiScene* scene, glmodel *model) {
	for(int i = 0; i < scene->mNumAnimations; i++) {
		glanimator_dl animator;
		aiAnimation* animation = scene->mAnimations[i];
		animator.m_CurrentTime = 0.0;
		for (int i = 0; i < 100; i++)
			animator.m_FinalBoneMatrices.push_back(mat4::identity());

		animator.m_Duration = animation->mDuration;
		animator.m_TicksPerSecond = animation->mTicksPerSecond;
		aiMatrix4x4 globalTransformation = scene->mRootNode->mTransformation;
		globalTransformation = globalTransformation.Inverse();
		readHeirarchyData(animator.m_RootNode, scene->mRootNode);

		for (int i = 0; i < animation->mNumChannels; i++) {
			aiNodeAnim* channel = animation->mChannels[i];
			string boneName = channel->mNodeName.C_Str();

			if (model->m_BoneInfoMap.count(boneName) == 0) {
				model->m_BoneInfoMap[boneName].id = model->m_BoneCounter;
				model->m_BoneCounter++;
			}
			glbone_dl b;
			b.m_Name = channel->mNodeName.C_Str();
			b.m_ID = model->m_BoneInfoMap[channel->mNodeName.data].id;
			b.m_LocalTransform = mat4::identity();
			for (int positionIndex = 0; positionIndex < channel->mNumPositionKeys; ++positionIndex)
			{
				aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
				float timeStamp = channel->mPositionKeys[positionIndex].mTime;
				KeyPositionDL data;
				data.position = vec3(aiPosition.x, aiPosition.y, aiPosition.z);
				data.timeStamp = timeStamp;
				b.m_Positions.push_back(data);
			}

			for (int rotationIndex = 0; rotationIndex < channel->mNumRotationKeys; ++rotationIndex)
			{
				aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
				float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
				KeyRotationDL data;
				data.orientation = quaternion(aiOrientation.w, vec3(aiOrientation.x, aiOrientation.y, aiOrientation.z));
				data.timeStamp = timeStamp;
				b.m_Rotations.push_back(data);
			}

			for (int keyIndex = 0; keyIndex < channel->mNumScalingKeys; ++keyIndex)
			{
				aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
				float timeStamp = channel->mScalingKeys[keyIndex].mTime;
				KeyScaleDL data;
				data.scale = vec3(scale.x, scale.y, scale.z);
				data.timeStamp = timeStamp;
				b.m_Scales.push_back(data);
			}
			animator.m_Bones.push_back(b);
		}
		animator.m_BoneInfoMap = model->m_BoneInfoMap;
		model->animator.push_back(animator);
	}
	return "";
}

string createMesh(const aiScene* scene, glmodel *model, string directory) {
	//Process Node Tree
	unordered_map<string, unsigned> loadedTexture;
	stringstream errStr;
			
	stack<aiNode*> nodeTreeStack;
	nodeTreeStack.push(scene->mRootNode);
	while(!nodeTreeStack.empty()) {
		//Select Current Node
		aiNode* node = nodeTreeStack.top();
		nodeTreeStack.pop();
		//Process All Meshes for Current Node
		for(unsigned int i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			glmesh_dl m;
			vector<Vertex> vertices;
			vector<unsigned int> indices;

			//Fill Vertices
			for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
				Vertex vertex;
				vertex.Position = vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
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
			errStr<<loadMaterialTextures(material, aiTextureType_DIFFUSE, directory, m.diffuseTextures);
			errStr<<loadMaterialTextures(material, aiTextureType_SPECULAR, directory, m.specularTextures);

			// Fill Bones and Weights
			for (int boneIndex = 0; boneIndex < mesh->mNumBones; ++boneIndex) {
				int boneID = -1;
				string boneName = mesh->mBones[boneIndex]->mName.C_Str();
				if (model->m_BoneInfoMap.count(boneName) == 0) {
					BoneInfoDL newBoneInfo;
					newBoneInfo.id = model->m_BoneCounter;
					newBoneInfo.offset = convertMatrixToVmathFormat(&mesh->mBones[boneIndex]->mOffsetMatrix);
					model->m_BoneInfoMap[boneName] = newBoneInfo;
					boneID = model->m_BoneCounter;
					model->m_BoneCounter++;
				} else {
					boneID = model->m_BoneInfoMap[boneName].id;
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

	return errStr.str();
}

glmodel::glmodel(string path, unsigned flags) {
	//Open File
	stringstream errStr;
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
		errStr<<createMesh(scene, this, path.substr(0, path.find_last_of('/')));
	}

	if(scene->HasAnimations()) {
		errStr<<createAnimator(scene, this);
	}
}

void calculateBoneTransform(glanimator_dl* a, const AssimpNodeDataDL* node, mat4 parentTransform) {
	string nodeName = node->name;
	mat4 nodeTransform = node->transformation;

	glbone_dl* Bone = findBone(a, nodeName);

	if(Bone) {	
		mat4 translationMat;
		mat4 rotationMat;
		mat4 scalingMat;
		int p0Index = -1;
		int p1Index = -1;

		//Calculate Translation
		if (Bone->m_Positions.size() == 1) {
			translationMat = translate(Bone->m_Positions[0].position);
		} else {
			for(p0Index = 0; p0Index < Bone->m_Positions.size() - 1; ++p0Index) {
				if (a->m_CurrentTime < Bone->m_Positions[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == Bone->m_Positions.size() - 1) {
				translationMat = translate(Bone->m_Positions[p0Index].position);
			} else {
				p1Index = p0Index + 1;
				translationMat = translate(mix(Bone->m_Positions[p0Index].position, Bone->m_Positions[p1Index].position, getScaleFactor(Bone->m_Positions[p0Index].timeStamp, Bone->m_Positions[p1Index].timeStamp, a->m_CurrentTime)));
			}
		}

		//Calculate Rotation
		if (Bone->m_Rotations.size() == 1) {
			rotationMat = quaternionToMatrix(normalize(Bone->m_Rotations[0].orientation));
		} else {
			for(p0Index = 0; p0Index < Bone->m_Rotations.size() - 1; ++p0Index) {
				if (a->m_CurrentTime < Bone->m_Rotations[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == Bone->m_Rotations.size() - 1) {
				rotationMat = quaternionToMatrix(normalize(Bone->m_Rotations[p0Index].orientation));
			} else {
				p1Index = p0Index + 1;
				rotationMat = quaternionToMatrix(normalize(slerp(Bone->m_Rotations[p0Index].orientation, Bone->m_Rotations[p1Index].orientation, getScaleFactor(Bone->m_Rotations[p0Index].timeStamp, Bone->m_Rotations[p1Index].timeStamp, a->m_CurrentTime))));
			}
		}

		//Calculate Scale
		if (Bone->m_Scales.size() == 1) {
			scalingMat = scale(Bone->m_Scales[0].scale);
		} else {
			for(p0Index = 0; p0Index < Bone->m_Scales.size() - 1; ++p0Index) {
				if (a->m_CurrentTime < Bone->m_Scales[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == Bone->m_Scales.size() - 1) {
				scalingMat = scale(Bone->m_Scales[p0Index].scale);
			} else {
				p1Index = p0Index + 1;
				scalingMat = scale(mix(Bone->m_Scales[p0Index].scale, Bone->m_Scales[p1Index].scale, getScaleFactor(Bone->m_Scales[p0Index].timeStamp, Bone->m_Scales[p1Index].timeStamp, a->m_CurrentTime)));
			}
		}

		Bone->m_LocalTransform = translationMat * rotationMat * scalingMat;
		nodeTransform = Bone->m_LocalTransform;
	}

	mat4 globalTransformation = parentTransform * nodeTransform;

	unordered_map<string, BoneInfoDL> boneInfoMap = a->m_BoneInfoMap;
	if (boneInfoMap.count(nodeName) != 0)
	{
		int index = boneInfoMap[nodeName].id;
		mat4 offset = boneInfoMap[nodeName].offset;
		a->m_FinalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++) {
		calculateBoneTransform(a, &node->children[i], globalTransformation);
	}
}

void glmodel::setBoneMatrixUniform(int i, int uniformLocation) {
	vector<mat4> m;
	if(i < this->animator.size() && i >= 0) {
		m = this->animator[i].m_FinalBoneMatrices;
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

string glmodel::update(float dt, int i) {
	if(i < this->animator.size()) {
		this->animator[i].m_DeltaTime = dt;
		this->animator[i].m_CurrentTime += this->animator[i].m_TicksPerSecond * dt;
		this->animator[i].m_CurrentTime = fmod(this->animator[i].m_CurrentTime, this->animator[i].m_Duration);
		calculateBoneTransform(&this->animator[i], &this->animator[i].m_RootNode, mat4::identity());
		return "";
	} else {
		return "Error: Animation not found\n";
	}
}

void glmodel::draw(int instance) {
	for(unsigned int i = 0; i < this->meshes.size(); i++) {
		if(this->meshes[i].diffuseTextures.size() > 0) {
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, this->meshes[i].diffuseTextures[0]);
		}
		if(this->meshes[i].specularTextures.size() > 0) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, this->meshes[i].specularTextures[0]);
		}
		glBindVertexArray(this->meshes[i].vao);
		glDrawElementsInstanced(GL_TRIANGLES, this->meshes[i].trianglePointCount, GL_UNSIGNED_INT, 0, instance);
	}
}
