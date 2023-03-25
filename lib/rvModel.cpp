#include <assert.h>
// OpenGL Header Files
#include<GL/glew.h> // this must be above gl.h
#include<GL/gl.h>

#include"vmath.h"
using namespace vmath;

// Image Loading Library
#include"stb_image.h"

#include <vector>
#include <string>
#include <map>

#include"../include/Model.hpp"

rvModel::rvModel()
{
    scene = NULL;
    hasAnimation = false;
    hasTexture = false;
}

rvModel::~rvModel()
{
    import.FreeScene();
}

void rvModel::ModelCleanUp()
{
    if(meshes.size() > 0)
    {
        for(size_t i = 0; i < meshes.size(); i++)
        {
            meshes[i].MeshCleanup();
        }
    }
}

void rvModel::initShaders(GLuint shader_program)
{
    for(int i = 0; i < MAX_BONES; i++)
    {
        std::string name = "gBones["+std::to_string(i)+"]";
        m_bone_location[i] = glGetUniformLocation(shader_program,name.c_str());
    }
}

void rvModel::draw(GLuint shader_program, double dt)
{
    if(hasAnimation)
    {
        std::vector<aiMatrix4x4> transforms;

        boneTransform(dt,transforms);

        for(size_t i = 0; i < transforms.size(); i++)
        {
			glUniformMatrix4fv(m_bone_location[i], 1, GL_TRUE, (const GLfloat*)&transforms[i]);
        }
    }

    for(size_t i = 0; i < meshes.size(); i++)
    {
        meshes[i].Draw(shader_program);
    }

}

void rvModel::drawInstanced(GLuint shader_program, double dt, GLint numOfInstances)
{
    if(hasAnimation)
    {
        std::vector<aiMatrix4x4> transforms;

        boneTransform(dt,transforms);

        for(size_t i = 0; i < transforms.size(); i++)
        {
            glUniformMatrix4fv(m_bone_location[i], 1, GL_TRUE, (const GLfloat*)&transforms[i]);
        }

        for(size_t i = 0; i < meshes.size(); i++)
        {
            meshes[i].DrawInstanced(shader_program,numOfInstances);
        }
    }
}

void rvModel::loadModel(const std::string& path)
{
    FILE *pFile = NULL;

    pFile = fopen("AssimpModelLogFile.txt","a+");
    fprintf(pFile,"Loading Model\n");
    fclose(pFile);

    scene = import.ReadFile(path,aiProcessPreset_TargetRealtime_Quality);

    if(!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        pFile = fopen("AssimpModelLogFile.txt","a+");
        fprintf(pFile,"Error::ASSIMP::MODEL::ReadFile Failed = %s\n",import.GetErrorString());
        fclose(pFile);
        return;
    }

    pFile = fopen("AssimpModelLogFile.txt","a+");
    fprintf(pFile,"Success::ASSIMP::MODEL::ReadFile Successs\n");
    fclose(pFile);

    hasAnimation = scene->HasAnimations();
    hasTexture = scene->HasTextures();

    directory = path.substr(0,path.find_last_of('/'));

    if(hasAnimation)
    {
        m_global_inverse_transform = scene->mRootNode->mTransformation;
        m_global_inverse_transform = m_global_inverse_transform.Inverse();

        if(scene->mAnimations[0]->mTicksPerSecond != 0.0)
        {
            ticks_per_second = (float) scene->mAnimations[0]->mTicksPerSecond;
        }else
        {
            ticks_per_second = 30.0f;
        }
    }

    pFile = fopen("AssimpModelLogFile.txt","a+");
	fprintf(pFile, "ASSIMP::MODEL:: Directory-> %s\n", directory.c_str());
    fprintf(pFile, "scene->mNumMeshes : %d\n", scene->mNumMeshes);
    fprintf(pFile, "scene->mNumMaterials : %d\n", scene->mNumMaterials);
    fprintf(pFile, "scene->HasTextures() : %s\n", hasTexture ? "True" : "False");
    if(hasTexture)
        fprintf(pFile, "scene->mNumTextures : %d\n", scene->mNumTextures);
	fprintf(pFile, "scene->HasAnimations() 1: %s\n", hasAnimation ? "True" : "False");
    if(hasAnimation)
    {
        fprintf(pFile, "scene->mAnimations[0]->mNumChannels : %d\n", scene->mAnimations[0]->mNumChannels);
        fprintf(pFile, "scene->mAnimations[0]->mDuration : %f\n", scene->mAnimations[0]->mDuration);
        fprintf(pFile, "scene->mAnimations[0]->mTicksPerSecond :  %f\n", scene->mAnimations[0]->mTicksPerSecond);
    }
	fprintf(pFile, "\tName nodes : \n");
	fclose(pFile);

    showNodeName(scene->mRootNode);

    processNode(scene->mRootNode,scene);

    if(hasAnimation)
    {
        for(size_t i = 0; i < scene->mNumAnimations; i++)
        {
            pFile = fopen("AssimpModelLogFile.txt","a+");
            fprintf(pFile,"Node::ANIMATION::NAME : %s\n",scene->mAnimations[0]->mName.C_Str());
            fclose(pFile);
			readNodeHierarchy(scene->mAnimations[0],scene->mRootNode);
        }
    }

	pFile = fopen("AssimpModelLogFile.txt","a+");
    fprintf(pFile,"Load Model Done Successfuly\n");
    fclose(pFile);

}

void rvModel::showNodeName(aiNode* node)
{
    FILE *pFile = NULL;
    pFile = fopen("AssimpModelLogFile.txt","a+");
    fprintf(pFile,"Data : = %s\n",node->mName.C_Str());
    fclose(pFile);

    for(size_t i = 0; i < node->mNumChildren; i++)
    {
        showNodeName(node->mChildren[i]);
    }
}

void rvModel::processNode(aiNode* node,const aiScene* scene)
{
    for(size_t i = 0; i < scene->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[i];
        meshes.push_back(processMesh(mesh,scene));
    }
}

void rvModel::readNodeHierarchy(const aiAnimation *p_animation, const aiNode *p_node)
{
	FILE* pFile;
	for(size_t i = 0; i <  p_animation->mNumChannels; i++)
	{
		aiNodeAnim* node_anim = p_animation->mChannels[i];
		std::string node_name(node_anim->mNodeName.data);

		if(m_nodeAnim_map.find(node_name) == m_nodeAnim_map.end())
		{
			m_nodeAnim_map[node_name] = node_anim;
		}
	}
}

rvMesh rvModel::processMesh(aiMesh* mesh,const aiScene* scene)
{
    FILE *pFile = NULL;
	std::vector<rvVertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<rvTexture> textures;
	std::vector<rvMaterial> mats;
    
    pFile = fopen("AssimpModelLogFile.txt","a+");
    fprintf(pFile,"Mesh Data : %s, Num Vertices : %d, indices %d Bones : %d.\n",mesh->mName.C_Str(),mesh->mNumVertices,(mesh->mNumFaces*3),mesh->mNumBones);
    fclose(pFile);

    for(size_t i = 0; i < mesh->mNumVertices; i++)
    {
        rvVertex vertex;
        vec3 vec;

		vec[0] = mesh->mVertices[i].x;
		vec[1] = mesh->mVertices[i].y;
		vec[2] = mesh->mVertices[i].z;

		vertex.Position = vec;

		if (mesh->HasNormals())
		{
			vec[0] = mesh->mNormals[i].x;
			vec[1] = mesh->mNormals[i].y;
			vec[2] = mesh->mNormals[i].z;
			vertex.Normal = vec;
		}

		if (mesh->mTextureCoords[0])
		{
			vec2 uv;

			uv[0] = mesh->mTextureCoords[0][i].x;
			uv[1] = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = uv;

			// Tangent
			vec[0] = mesh->mTangents[i].x;
			vec[1] = mesh->mTangents[i].y;
			vec[2] = mesh->mTangents[i].z;
			vertex.Tangent = vec;

			vec[0] = mesh->mBitangents[i].x;
			vec[1] = mesh->mBitangents[i].y;
			vec[2] = mesh->mBitangents[i].z;
			vertex.Bitangent = vec;
		}
		else
			vertex.TexCoords = vec2(0.0f, 0.0f);
		vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

    if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        pFile = fopen("AssimpModelLogFile.txt","a+");
        fprintf(pFile,"Material Name : %s\n",material->GetName().C_Str());
        fclose(pFile);

		//diffuse map
		std::vector<rvTexture> diffuseMap = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMap.begin(), diffuseMap.end());

		//specular map
		std::vector<rvTexture> specularMap = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMap.begin(), specularMap.end());

		//normal map
		std::vector<rvTexture> normalMap = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
		textures.insert(textures.end(), normalMap.begin(), normalMap.end());

		// emissive
		std::vector<rvTexture> emissiveMap = loadMaterialTextures(material, aiTextureType_EMISSIVE, "texture_emissive");
		textures.insert(textures.end(), emissiveMap.begin(), emissiveMap.end());

		std::vector<rvMaterial> MaterialAmbient = loadMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, "material_ambient_animModel");
		mats.insert(mats.end(),MaterialAmbient.begin(), MaterialAmbient.end());

		std::vector<rvMaterial> MaterialDiffuse = loadMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, "material_diffuse_animModel");
		mats.insert(mats.end(),MaterialDiffuse.begin(), MaterialDiffuse.end());

		std::vector<rvMaterial> MaterialSpecular = loadMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, "material_specular_animModel");
		mats.insert(mats.end(),MaterialSpecular.begin(), MaterialSpecular.end());
	}

    // Loadbones
    if(hasAnimation && mesh->HasBones())
    {
        for (int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++)
		{
			int bone_index = 0;
			std::string bone_name(mesh->mBones[boneIndex]->mName.data);

            
            //fopen_s(&pFile,"AssimpModelLogFile.txt","a+");
            //fprintf(pFile,"Model::Bone::Name  %s\n",mesh->mBones[boneIndex]->mName.data);
            //fclose(pFile);

            if(m_bone_mapping.find(bone_name) == m_bone_mapping.end())
            {
                bone_index = m_BoneCounter;
                m_BoneCounter++;
                rvBoneMatrix bi;
                m_bone_matrices.push_back(bi);
				m_bone_matrices[bone_index].aimat44_Offset_Matrix = mesh->mBones[boneIndex]->mOffsetMatrix;
                m_bone_mapping[bone_name] = bone_index;
            }
            else
            {
                bone_index = m_bone_mapping[bone_name];
            }

            for(size_t j = 0; j < mesh->mBones[boneIndex]->mNumWeights; j++)
            {
                int vertex_id = mesh->mBones[boneIndex]->mWeights[j].mVertexId;
                float weights = mesh->mBones[boneIndex]->mWeights[j].mWeight;
                vertices[vertex_id].SetVertexBoneData(bone_index, weights);
            }
		}
    }
    return rvMesh(vertices,indices,textures,mats);
}

std::vector<rvTexture> rvModel::loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typestring)
{
	std::vector<rvTexture> textures;

	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);

		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}

		if (!skip)
		{
			rvTexture texture;
			texture.id = TextureFromFile(str.C_Str(), directory);
			texture.type = typestring;
            texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}
	return textures;
}

std::vector<rvMaterial> rvModel::loadMaterialColor(aiMaterial *mat, const char *type, int one, int two, std::string typeString)
{
    std::vector<rvMaterial> materials;

	aiColor3D color;
	mat->Get(type, one, two, color);

	rvMaterial matInfo;
	matInfo.type = typeString;
	matInfo.value[0] = color[0];
	matInfo.value[1] = color[1];
	matInfo.value[2] = color[2];

    FILE *pFile = NULL;

    //fopen_s(&pFile,"AssimpModelLogFile.txt","a+");
	//fprintf(pFile,"Material %s : value = %.1f, %.1f, %.1f\n",matInfo.type.c_str(), matInfo.value[0], matInfo.value[1], matInfo.value[2]);
    //fclose(pFile);

	materials.push_back(matInfo);
	return materials;
}

int rvModel::findPosition(float p_animation_time,const aiNodeAnim* p_node_anim)
{
    for(size_t i = 0; i < p_node_anim->mNumPositionKeys - 1; i++)
    {
        if(p_animation_time < (float)p_node_anim->mPositionKeys[i+1].mTime)
        {
            return i;
        }
    }
    return 0;
}

int rvModel::findRotation(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    for(size_t i = 0; i < p_node_anim->mNumRotationKeys - 1; i++)
    {
        if(p_animation_time < (float)p_node_anim->mRotationKeys[i+1].mTime)
            return i;
    }
    return 0;
}

int rvModel::findScaling(float p_animation_time, const aiNodeAnim* p_node_anim)
{
    for(size_t i = 0; i < p_node_anim->mNumScalingKeys - 1; i++)
    {
        if(p_animation_time < (float)p_node_anim->mScalingKeys[i+1].mTime)
        {
            return i;
        }
    }
    return 0;
}

const aiNodeAnim* rvModel::findNodeAnim(const aiAnimation* p_animation, const std::string p_node_name)
{

	if(m_nodeAnim_map.find(p_node_name) != m_nodeAnim_map.end())
		return m_nodeAnim_map[p_node_name];
	else
		return nullptr;
/*
    for(size_t i = 0; i < p_animation->mNumChannels;i++)
    {
        const aiNodeAnim* node_anim = p_animation->mChannels[i];
        if(std::string(node_anim->mNodeName.data) == p_node_name)
        {
            return node_anim;
        }
    }
    return nullptr;
	*/
}

aiVector3D rvModel::calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	if (p_node_anim->mNumPositionKeys == 1)
	{
		return p_node_anim->mPositionKeys[0].mValue;
	}

	unsigned int position_index = findPosition(p_animation_time, p_node_anim);
	unsigned int next_position_index = position_index + 1;

	assert(next_position_index < p_node_anim->mNumPositionKeys);

	float delta_time = (float)(p_node_anim->mPositionKeys[next_position_index].mTime - p_node_anim->mPositionKeys[position_index].mTime);

	float factor = (p_animation_time - (float)p_node_anim->mPositionKeys[position_index].mTime) / delta_time;

	assert(factor >= 0.0f && factor <= 1.0f);
	aiVector3D start = p_node_anim->mPositionKeys[position_index].mValue;
	aiVector3D end = p_node_anim->mPositionKeys[next_position_index].mValue;
	aiVector3D delta = end - start;

	return start + factor * delta;
}

aiQuaternion rvModel::calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	if (p_node_anim->mNumRotationKeys == 1)
	{
		return p_node_anim->mRotationKeys[0].mValue;
	}

	unsigned int rotation_index = findRotation(p_animation_time, p_node_anim);
	unsigned int next_rotation_index = rotation_index + 1;
	assert(next_rotation_index < p_node_anim->mNumRotationKeys);

	float delta_time = (float)(p_node_anim->mRotationKeys[next_rotation_index].mTime - p_node_anim->mRotationKeys[rotation_index].mTime);

	float factor = (p_animation_time - (float)p_node_anim->mRotationKeys[rotation_index].mTime) / delta_time;

	assert(factor >= 0.0f && factor <= 1.0f);
	aiQuaternion start_quat = p_node_anim->mRotationKeys[rotation_index].mValue;
	aiQuaternion end_quat = p_node_anim->mRotationKeys[next_rotation_index].mValue;

	return nlerp(start_quat, end_quat, factor);
}

aiVector3D rvModel::calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim)
{
	if (p_node_anim->mNumScalingKeys == 1)
	{
		return p_node_anim->mScalingKeys[0].mValue;
	}

	unsigned int scaling_index = findScaling(p_animation_time, p_node_anim);
	unsigned int next_scaling_index = scaling_index + 1;
	assert(next_scaling_index < p_node_anim->mNumScalingKeys);

	float delta_time = (float)(p_node_anim->mScalingKeys[next_scaling_index].mTime - p_node_anim->mScalingKeys[scaling_index].mTime);

	float  factor = (p_animation_time - (float)p_node_anim->mScalingKeys[scaling_index].mTime) / delta_time;
	assert(factor >= 0.0f && factor <= 1.0f);
	aiVector3D start = p_node_anim->mScalingKeys[scaling_index].mValue;
	aiVector3D end = p_node_anim->mScalingKeys[next_scaling_index].mValue;
	aiVector3D delta = end - start;

	return start + factor * delta;
}

void rvModel::readNodeHierarchy(float p_animation_time, const aiNode* p_node,const aiMatrix4x4 parent_transform)
{
	std::string node_name(p_node->mName.data);

	const aiAnimation* animation = scene->mAnimations[0];
	aiMatrix4x4 node_transform = p_node->mTransformation;

	const aiNodeAnim* node_anim = findNodeAnim(animation, node_name);
	if (node_anim)
	{

		//scaling
		//aiVector3D scaling_vector = node_anim->mScalingKeys[2].mValue;
		aiVector3D scaling_vector = calcInterpolatedScaling(p_animation_time, node_anim);
		aiMatrix4x4 scaling_matr;
		aiMatrix4x4::Scaling(scaling_vector, scaling_matr);

		//rotation
		//aiQuaternion rotate_quat = node_anim->mRotationKeys[2].mValue;
		aiQuaternion rotate_quat = calcInterpolatedRotation(p_animation_time, node_anim);
		aiMatrix4x4 rotate_matr = aiMatrix4x4(rotate_quat.GetMatrix());

		//translation
		//aiVector3D translate_vector = node_anim->mPositionKeys[2].mValue;
		aiVector3D translate_vector = calcInterpolatedPosition(p_animation_time, node_anim);
		aiMatrix4x4 translate_matr;
		aiMatrix4x4::Translation(translate_vector, translate_matr);
		node_transform = translate_matr * rotate_matr * scaling_matr;
	}

	aiMatrix4x4 global_transform = parent_transform * node_transform;

	if (m_bone_mapping.find(node_name) != m_bone_mapping.end()) // true if node_name exist in bone_mapping
	{
		int bone_index = m_bone_mapping[node_name];
		m_bone_matrices[bone_index].aimat44_Final_World_Transform = global_transform * m_bone_matrices[bone_index].aimat44_Offset_Matrix;
	}

	for (unsigned int i = 0; i < p_node->mNumChildren; i++)
	{
		readNodeHierarchy(p_animation_time, p_node->mChildren[i], global_transform);
	}
}

void rvModel::boneTransform(double time_in_sec, std::vector<aiMatrix4x4>& transforms)
{
	aiMatrix4x4 identity_matrix; // = mat4(1.0f);

	double time_in_ticks = time_in_sec * ticks_per_second;
	float animation_time = (float)fmod(time_in_ticks, scene->mAnimations[0]->mDuration);

	readNodeHierarchy(animation_time, scene->mRootNode, identity_matrix);

	transforms.resize(m_BoneCounter);

	for (unsigned int i = 0; i < m_BoneCounter; i++)
	{
		transforms[i] = m_bone_matrices[i].aimat44_Final_World_Transform;
	}
}

aiQuaternion rvModel::nlerp(aiQuaternion a, aiQuaternion b, float blend)
{

	a.Normalize();
	b.Normalize();

	aiQuaternion result;
	float dot_product = a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
	float one_minus_blend = 1.0f - blend;

	if (dot_product < 0.0f)
	{
		result.x = a.x * one_minus_blend + blend * -b.x;
		result.y = a.y * one_minus_blend + blend * -b.y;
		result.z = a.z * one_minus_blend + blend * -b.z;
		result.w = a.w * one_minus_blend + blend * -b.w;
	}
	else
	{
		result.x = a.x * one_minus_blend + blend * b.x;
		result.y = a.y * one_minus_blend + blend * b.y;
		result.z = a.z * one_minus_blend + blend * b.z;
		result.w = a.w * one_minus_blend + blend * b.w;
	}

	return result.Normalize();
}

unsigned int TextureFromFile(const char* path, const std::string& directory)
{
	stbi_set_flip_vertically_on_load(true);
	std::string filename = std::string(path);
	filename = directory + '/' + filename;
    FILE* pFile;

    pFile = fopen("AssimpModelLogFile.txt","a+");
	fprintf(pFile, "Texture path :: %s\n", filename.c_str());
    fclose(pFile);

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}
	stbi_image_free(data);
	return textureID;
}
