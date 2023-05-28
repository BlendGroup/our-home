#include <X11/Xlib.h>
#include <algorithm>
#include <assimp/material.h>
#include <assimp/types.h>
#include <assimp/vector3.h>
#include <cmath>
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
#include<glshaderloader.h>
#include<gltextureloader.h>
#include<vmath.h>
#include<glmodelloader.h>
#include<errorlog.h>

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

unordered_map<textureTypes, string> textureTypeMap = {
	{TEX_DIFFUSE, "texture_diffuse"},
	{TEX_NORMAL, "texture_normal"},
	{TEX_SPECULAR, "texture_specular"},
	{TEX_EMISSIVE,"texture_emissive"},
	{TEX_PBR,"texture_PBR"}
};

unordered_map<materialTypes, string> materialTypeMap = {
    {MAT_AMBIENT,"material.ambient"},
    {MAT_DIFFUSE,"material.diffuse"},
    {MAT_SPECULAR,"material.specular"},
	{MAT_EMISSIVE,"material.emissive"},
    {MAT_METALLIC,"material.metallic"},
	{MAT_ROUGHNESS,"material.roughness"},
    {MAT_OPACITY,"material.opacity"},
	{MAT_SPECULAR_INTENSITY,"material.specular_intensity"},
	{MAT_SPECULAR_POWER,"material.specular_power"}
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

vector<texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, textureTypes typeString,string directory) {
	vector<texture> textures;
    for(size_t i = 0; i < mat->GetTextureCount(type); i++){
        aiString str;
        mat->GetTexture(type, i, &str);
		texture tex;
        tex.id = createTexture2D(directory + "/" + str.C_Str(),GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
        tex.type = typeString;
        textures.push_back(tex);
    }
    return textures;
}

texture loadPBRTextures(textureTypes typeString,string directory,string matName) {
	texture tex;
	#ifdef DEBUG
	cout<<directory + "/textures/" + matName+"_"+textureTypeMap[typeString]+".jpg"<<endl;
	#endif
	if(isTexturePresent(directory + "/textures/" + matName+"_"+textureTypeMap[typeString]+".jpg")) {
		tex.id = createTexture2D(directory + "/textures/" + matName+"_"+textureTypeMap[typeString]+".jpg",GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, GL_REPEAT, GL_REPEAT);
		tex.type = typeString;
	} else {
		tex.id = 0;
		tex.type = TEX_DIFFUSE;
	}
	return tex;
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
			if(mesh->mMaterialIndex >= 0)
			{
				m.materialIndex = mesh->mMaterialIndex;
			}

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

glmodel::glmodel(string path, unsigned flags, bool isPbr) {
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

	this->PBR = isPbr;
	if(scene->HasMaterials()){
		for(size_t m = 0; m < scene->mNumMaterials; m++){

			glmaterial temp;
			aiMaterial* mat = scene->mMaterials[m];

			aiString name;
			mat->Get(AI_MATKEY_NAME,name);
			aiColor3D color;			
			mat->Get(AI_MATKEY_COLOR_AMBIENT,color);
			temp.ambient[0] = color[0];
			temp.ambient[1] = color[1];
			temp.ambient[2] = color[2];

			mat->Get(AI_MATKEY_COLOR_DIFFUSE,color);
			temp.diffuse[0] = color[0];
			temp.diffuse[1] = color[1];
			temp.diffuse[2] = color[2];

			mat->Get(AI_MATKEY_COLOR_SPECULAR,color);
			temp.specular[0] = color[0];
			temp.specular[1] = color[1];
			temp.specular[2] = color[2];

			mat->Get(AI_MATKEY_COLOR_EMISSIVE,color);
			temp.emissive[0] = color[0];
			temp.emissive[1] = color[1];
			temp.emissive[2] = color[2];

			float value;
			mat->Get(AI_MATKEY_OPACITY,value);
			temp.opacity = value;
			
			if(isPbr){

				// Means Metallic and Roughness Work flow
				if(aiReturn_SUCCESS == mat->Get(AI_MATKEY_METALLIC_FACTOR,value))
				{
					temp.metallic = value;
					if(aiReturn_SUCCESS == mat->Get(AI_MATKEY_ROUGHNESS_FACTOR,value))
						temp.roughness = value;
				}
				// Means Specular and glossiness factor
				else {
					temp.metallic = max(temp.specular[0], max(temp.specular[1],temp.specular[2]));
					mat->Get(AI_MATKEY_GLOSSINESS_FACTOR,value);
					temp.roughness = value;
				}

				// Load all PBR Textures Manually as assimp is a bitch
				texture tempTex;
				
				tempTex = loadPBRTextures(TEX_DIFFUSE, path.substr(0, path.find_last_of('/')),name.data);
				if(tempTex.id != 0) temp.textures.push_back(tempTex);

				tempTex = loadPBRTextures(TEX_NORMAL, path.substr(0, path.find_last_of('/')),name.data);
				if(tempTex.id != 0) temp.textures.push_back(tempTex);

				tempTex = loadPBRTextures(TEX_PBR, path.substr(0, path.find_last_of('/')),name.data);
				if(tempTex.id != 0) temp.textures.push_back(tempTex);

				tempTex = loadPBRTextures(TEX_SPECULAR, path.substr(0, path.find_last_of('/')),name.data);
				if(tempTex.id != 0) temp.textures.push_back(tempTex);

				tempTex = loadPBRTextures(TEX_EMISSIVE, path.substr(0, path.find_last_of('/')),name.data);
				if(tempTex.id != 0) temp.textures.push_back(tempTex);

			}else {

				mat->Get(AI_MATKEY_SHININESS,value);	
				temp.metallic = value;
				mat->Get(AI_MATKEY_SPECULAR_FACTOR,value);
				temp.roughness = value;

				temp.textures.push_back(loadPBRTextures(TEX_DIFFUSE, path.substr(0, path.find_last_of('/')),name.data));
				temp.textures.push_back(loadPBRTextures(TEX_NORMAL, path.substr(0, path.find_last_of('/')),name.data));
				temp.textures.push_back(loadPBRTextures(TEX_EMISSIVE, path.substr(0, path.find_last_of('/')),name.data));
			}
			this->materials.push_back(temp);
		}
	}

	if(scene->HasAnimations()) {
		createAnimator(scene, this);
	}
/*
	cout<<"Animation :: "<<this->animator.size()<<endl;
	for(auto a : this->animator){
		cout<<a.duration<<endl;
		cout<<a.ticksPerSecond<<endl;
		cout<<a.bones.size()<<endl;
		cout<<a.rootNode.name<<endl<<endl;
	}
*/
	cout<<"material count"<<this->materials.size()<<endl;
	for(auto m : materials)
	{
		cout<<"ambient : "<<m.ambient[0]<<m.ambient[1]<<m.ambient[2]<<endl;
		cout<<"diffuse : "<<m.diffuse[0]<<m.diffuse[1]<<m.diffuse[2]<<endl;
		cout<<"specular : "<<m.specular[0]<<m.specular[1]<<m.specular[2]<<endl;
		cout<<"emissive : "<<m.emissive[0]<<m.emissive[1]<<m.emissive[2]<<endl;
		cout<<"metallic : "<<m.metallic<<" roughness : "<<m.roughness<<endl;
		cout<<"opacity : "<<m.opacity<<endl;
		cout<<"tex count : "<<m.textures.size()<<endl;
		for(auto t : m.textures){
			cout<<t.id<<textureTypeMap[t.type]<<endl;
		}
	}

	importer.FreeScene();
}
void calculateBoneTransformBlended(glmodel* model, 
glanimator* base,const AssimpNodeData* node,
glanimator* layer,const AssimpNodeData* nodeLayer,
const float currentTimeBase, const float currentTimeLayered,
mat4 parentTransform,float blendFactor) {

	string nodeName = node->name;
	mat4 nodeTransform = node->transformation;

	glbone* startBone = findBone(base, nodeName);

	if(startBone) {
		mat4 translationMat;
		mat4 rotationMat;
		mat4 scalingMat;
		int p0Index = -1;
		int p1Index = -1;

		//Calculate Translation
		if (startBone->positions.size() == 1) {
			translationMat = translate(startBone->positions[0].position);
		} else {
			for(p0Index = 0; p0Index < startBone->positions.size() - 1; ++p0Index) {
				if (currentTimeBase < startBone->positions[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == startBone->positions.size() - 1) {
				translationMat = translate(startBone->positions[p0Index].position);
			} else {
				p1Index = p0Index + 1;
				translationMat = translate(mix(startBone->positions[p0Index].position, startBone->positions[p1Index].position, getScaleFactor(startBone->positions[p0Index].timeStamp, startBone->positions[p1Index].timeStamp, currentTimeBase)));
			}
		}

		//Calculate Rotation
		if (startBone->rotations.size() == 1) {
			rotationMat = quaternionToMatrix(normalize(startBone->rotations[0].orientation));
		} else {
			for(p0Index = 0; p0Index < startBone->rotations.size() - 1; ++p0Index) {
				if (currentTimeBase < startBone->rotations[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == startBone->rotations.size() - 1) {
				rotationMat = quaternionToMatrix(normalize(startBone->rotations[p0Index].orientation));
			} else {
				p1Index = p0Index + 1;
				rotationMat = quaternionToMatrix(normalize(slerp(startBone->rotations[p0Index].orientation, startBone->rotations[p1Index].orientation, getScaleFactor(startBone->rotations[p0Index].timeStamp, startBone->rotations[p1Index].timeStamp, currentTimeBase))));
			}
		}

		//Calculate Scale
		if (startBone->scales.size() == 1) {
			scalingMat = scale(startBone->scales[0].scale);
		} else {
			for(p0Index = 0; p0Index < startBone->scales.size() - 1; ++p0Index) {
				if (currentTimeBase < startBone->scales[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == startBone->scales.size() - 1) {
				scalingMat = scale(startBone->scales[p0Index].scale);
			} else {
				p1Index = p0Index + 1;
				vec3 s = mix(startBone->scales[p0Index].scale, startBone->scales[p1Index].scale, getScaleFactor(startBone->scales[p0Index].timeStamp, startBone->scales[p1Index].timeStamp, currentTimeBase));
				scalingMat = scale(s);
			}
		}
		startBone->localTransform = translationMat * rotationMat * scalingMat;
		nodeTransform = startBone->localTransform;
	}

	mat4 layeredNodeTransform = nodeLayer->transformation;

	glbone* endBone = findBone(layer, nodeName);

	if(endBone) {
		mat4 translationMat;
		mat4 rotationMat;
		mat4 scalingMat;
		int p0Index = -1;
		int p1Index = -1;

		//Calculate Translation
		if (endBone->positions.size() == 1) {
			translationMat = translate(endBone->positions[0].position);
		} else {
			for(p0Index = 0; p0Index < endBone->positions.size() - 1; ++p0Index) {
				if (currentTimeLayered < endBone->positions[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == endBone->positions.size() - 1) {
				translationMat = translate(endBone->positions[p0Index].position);
			} else {
				p1Index = p0Index + 1;
				translationMat = translate(mix(endBone->positions[p0Index].position, endBone->positions[p1Index].position, getScaleFactor(endBone->positions[p0Index].timeStamp, endBone->positions[p1Index].timeStamp, currentTimeLayered)));
			}
		}

		//Calculate Rotation
		if (endBone->rotations.size() == 1) {
			rotationMat = quaternionToMatrix(normalize(endBone->rotations[0].orientation));
		} else {
			for(p0Index = 0; p0Index < endBone->rotations.size() - 1; ++p0Index) {
				if (currentTimeLayered < endBone->rotations[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == endBone->rotations.size() - 1) {
				rotationMat = quaternionToMatrix(normalize(endBone->rotations[p0Index].orientation));
			} else {
				p1Index = p0Index + 1;
				rotationMat = quaternionToMatrix(normalize(slerp(endBone->rotations[p0Index].orientation, endBone->rotations[p1Index].orientation, getScaleFactor(endBone->rotations[p0Index].timeStamp, endBone->rotations[p1Index].timeStamp, currentTimeLayered))));
			}
		}

		//Calculate Scale
		if (endBone->scales.size() == 1) {
			scalingMat = scale(endBone->scales[0].scale);
		} else {
			for(p0Index = 0; p0Index < endBone->scales.size() - 1; ++p0Index) {
				if (currentTimeLayered < endBone->scales[p0Index + 1].timeStamp) {
					break;
				}
			}
			if(p0Index == endBone->scales.size() - 1) {
				scalingMat = scale(endBone->scales[p0Index].scale);
			} else {
				p1Index = p0Index + 1;
				vec3 s = mix(endBone->scales[p0Index].scale, endBone->scales[p1Index].scale, getScaleFactor(endBone->scales[p0Index].timeStamp, endBone->scales[p1Index].timeStamp, currentTimeLayered));
				scalingMat = scale(s);
			}
		}
		endBone->localTransform = translationMat * rotationMat * scalingMat;
		layeredNodeTransform = endBone->localTransform;
	}

/*
	quaternion rot0 = quaternion(nodeTransform);
	quaternion rot1 = quaternion(layeredNodeTransform);
	quaternion finalRot = slerp(rot0, rot1, blendFactor);
	mat4 blendedMat = quaternionToMatrix(finalRot);
	blendedMat[3] = (1.0f - blendFactor) * nodeTransform[3] + layeredNodeTransform[3] * blendFactor;
*/
	mat4 blendedMat = (nodeTransform * (1-blendFactor)) + (layeredNodeTransform * blendFactor);
	mat4 globalTransformation = parentTransform * blendedMat;

	unordered_map<string, BoneInfo> boneInfoMap = model->boneInfoMap;
	if (boneInfoMap.count(nodeName) != 0)
	{
		int index = boneInfoMap[nodeName].id;
		mat4 offset = boneInfoMap[nodeName].offset;
		base->finalBoneMatrices[index] = globalTransformation * offset;
		layer->finalBoneMatrices[index] = globalTransformation * offset;
	}

	for (int i = 0; i < node->childrenCount; i++) {
		calculateBoneTransformBlended(model, base, &node->children[i], layer, &nodeLayer->children[i], currentTimeBase, currentTimeLayered, globalTransformation, blendFactor);
	}
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
				vec3 s = mix(bone->scales[p0Index].scale, bone->scales[p1Index].scale, getScaleFactor(bone->scales[p0Index].timeStamp, bone->scales[p1Index].timeStamp, a->currentTime));
				scalingMat = scale(s);
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

void BlendTwoAnimations(glmodel* model,glanimator* baseAnimation,glanimator* layeredAnimation,float blendFactor,float dt){

	float a = 1.0f;
	float b = baseAnimation->duration / layeredAnimation->duration;
	const float animSpeedMultiplierUp = (1.0f - blendFactor) * a + b * blendFactor;

	a = layeredAnimation->duration / baseAnimation->duration;
	b = 1.0f;
	const float animSpeedMultiplierDown = (1.0f - blendFactor) * a + b * blendFactor;

	static float currentTimeBase = 0.0f;
	currentTimeBase += baseAnimation->ticksPerSecond * dt * animSpeedMultiplierUp;
	currentTimeBase = fmod(currentTimeBase,baseAnimation->duration);

	static float currentTimeLayered = 0.0f;
	currentTimeLayered += layeredAnimation->ticksPerSecond * dt * animSpeedMultiplierDown;
	currentTimeLayered = fmod(currentTimeLayered,layeredAnimation->duration);

	calculateBoneTransformBlended(model,baseAnimation,&baseAnimation->rootNode,layeredAnimation,&layeredAnimation->rootNode,currentTimeBase,currentTimeLayered,mat4::identity(),blendFactor);
}


void glmodel::setBoneMatrixUniform(GLuint uniformLocation, unsigned i) {
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

void glmodel::update(float dt, int baseAnimation , int layeredAnimation , float blendFactor) {
	if(baseAnimation == layeredAnimation && baseAnimation < this->animator.size()) {
		this->animator[baseAnimation].currentTime += this->animator[baseAnimation].ticksPerSecond * dt;
		this->animator[baseAnimation].currentTime = fmod(this->animator[baseAnimation].currentTime, this->animator[baseAnimation].duration);
		calculateBoneTransform(this, &this->animator[baseAnimation], &this->animator[baseAnimation].rootNode, mat4::identity());
	}
	else {
		BlendTwoAnimations(this,&this->animator[baseAnimation],&this->animator[layeredAnimation],blendFactor,dt);
	}
}

void glmodel::draw(glshaderprogram *program,int instance,bool setMaterials) {
	try {
		for(unsigned int i = 0; i < this->meshes.size(); i++) {
			
			//Set Material Uniforms  Can't Do This Outside of Draw !!!!
			if(setMaterials)
			{
				//setup textures	
				if(this->materials[this->meshes[i].materialIndex].textures.size() > 0)
				{
					glUniform1i(program->getUniformLocation("isTexture"),true);
					for(int t = 0; t < this->materials[this->meshes[i].materialIndex].textures.size(); t++)
					{
						glBindTextureUnit(this->materials[this->meshes[i].materialIndex].textures[t].type,this->materials[this->meshes[i].materialIndex].textures[t].id);
					}
				}else {
					glUniform1i(program->getUniformLocation("isTexture"),false);
					glUniform3fv(program->getUniformLocation(materialTypeMap[MAT_DIFFUSE]),1,this->materials[this->meshes[i].materialIndex].diffuse);
					glUniform3fv(program->getUniformLocation(materialTypeMap[MAT_EMISSIVE]),1,this->materials[this->meshes[i].materialIndex].emissive);
				}		
				if(PBR){
					glUniform1f(program->getUniformLocation(materialTypeMap[MAT_METALLIC]),this->materials[this->meshes[i].materialIndex].metallic);
					glUniform1f(program->getUniformLocation(materialTypeMap[MAT_ROUGHNESS]),this->materials[this->meshes[i].materialIndex].roughness);
				}else {
					glUniform1f(program->getUniformLocation(materialTypeMap[MAT_SPECULAR_POWER]),this->materials[this->meshes[i].materialIndex].metallic);
					glUniform1f(program->getUniformLocation(materialTypeMap[MAT_SPECULAR_INTENSITY]),this->materials[this->meshes[i].materialIndex].roughness);
				}
				glUniform1f(program->getUniformLocation(materialTypeMap[MAT_OPACITY]),this->materials[this->meshes[i].materialIndex].opacity);
				
				//TEMP
				// glUniform1i(program->getUniformLocation("texture_diffuse"), 0);
				// glBindTextureUnit(0, this->materials[this->meshes[i].materialIndex].textures[TEX_DIFFUSE].id);
				//////
			}
			glBindVertexArray(this->meshes[i].vao);
			glDrawElementsInstanced(GL_TRIANGLES, this->meshes[i].trianglePointCount, GL_UNSIGNED_INT, 0, instance);
			// unbind textures
			glBindTextureUnit(0,0);
			glBindTextureUnit(1,0);
			glBindTextureUnit(2,0);
			glBindTextureUnit(3,0);
			glBindTextureUnit(4,0);
			glBindVertexArray(0);
		}
	}
	catch(string errString) {
		throwErr(errString);
	}
}
