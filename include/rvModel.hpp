#pragma once

#include <assert.h>
#include <assimp/texture.h>
#include <map>
#include <vector>

#include<GL/glew.h> // this must be above gl.h

#include<GL/gl.h>

#include"vmath.h"
using namespace vmath;

// Image Loading Library
#include"stb_image.h"

#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>
#include<assimp/DefaultLogger.hpp>
#include<assimp/LogStream.hpp>

#include"rvMesh.hpp"

#define MAX_BONES   100
GLuint TextureFromFile(const char* path, const std::string& directory);

class rvModel
{
    private:
        Assimp::Importer import;
        const aiScene* scene;
        std::vector<rvMesh> meshes;
        std::vector<rvTexture> textures_loaded;
        std::string directory;
        std::map<std::string, unsigned int> m_bone_mapping;
        int m_BoneCounter = 0;

        std::vector<rvBoneMatrix> m_bone_matrices;
        aiMatrix4x4 m_global_inverse_transform;

        unsigned int m_bone_location[MAX_BONES];
        float ticks_per_second = 0.0f;

        std::map<std::string,aiNodeAnim*> m_nodeAnim_map;

        bool hasAnimation,hasTexture;

        // Utility functions

        void processNode(aiNode* node, const aiScene* scene);
        void readNodeHierarchy(const aiAnimation* p_animation,const aiNode* p_node);
        rvMesh processMesh(aiMesh* mesh,const aiScene* scene);
        std::vector<rvTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typestring);
        std::vector<rvMaterial> loadMaterialColor(aiMaterial* mat, const char* type, int one, int two, std::string typeString);

        int findPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
        int findRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
        int findScaling(float p_animation_time, const aiNodeAnim* p_node_anim);

        const aiNodeAnim* findNodeAnim(const aiAnimation* p_animation, const std::string p_node_name);

        // calculate transform matrix
        aiVector3D calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
        aiQuaternion calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
        aiVector3D calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim);

        void readNodeHierarchy(float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform);
        void boneTransform(double time_in_sec, std::vector<aiMatrix4x4>& transforms);

    public:

        rvModel();
        ~rvModel();

        void ModelCleanUp();

        void initShaders(GLuint shader_program);
        void loadModel(const std::string& path);

        // render
        void draw(GLuint shader_program,double dt);
        void drawInstanced(GLuint shader_program, double dt, GLint numOfInstances);

        void showNodeName(aiNode* node);

        aiQuaternion nlerp(aiQuaternion a, aiQuaternion b, float blend);
};