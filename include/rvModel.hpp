#pragma once

#include <assert.h>
#include <assimp/anim.h>
#include <assimp/matrix4x4.h>
#include <assimp/texture.h>
#include <map>
#include <string>
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

struct rvAnimation
{
    float duration;
    float m_TicksPerSecond;
    std::map<std::string,aiNodeAnim*> m_nodeAnim_map;
    void readNodeHierarchy(const aiAnimation* p_animation,const aiNode* p_node);
    void readMissingBones(const aiAnimation* p_animation,std::map<std::string, unsigned int>& m_bone_mapping,int& m_BoneCounter,std::vector<rvBoneMatrix> &m_bone_matrices);
};

class rvModel
{
    private:
        Assimp::Importer import;
        const aiScene* scene;
        aiNode* mSceneRoot;
        std::vector<rvMesh> meshes;
        std::vector<rvTexture> textures_loaded;
        std::vector<rvAnimation> animations;
        std::string directory;
        std::map<std::string, unsigned int> m_bone_mapping;
        int m_BoneCounter = 0;

        std::vector<rvBoneMatrix> m_bone_matrices;
        aiMatrix4x4 m_global_inverse_transform;

        unsigned int m_bone_location[MAX_BONES];
        float ticks_per_second = 0.0f;

        bool hasAnimation,hasTexture;

        // Utility functions

        void processNode(aiNode* node, const aiScene* scene);
        rvMesh processMesh(aiMesh* mesh,const aiScene* scene);
        std::vector<rvTexture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typestring);
        std::vector<rvMaterial> loadMaterialColor(aiMaterial* mat, const char* type, int one, int two, std::string typeString);

        int findPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
        int findRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
        int findScaling(float p_animation_time, const aiNodeAnim* p_node_anim);

        const aiNodeAnim* findNodeAnim(int animationIndex,const std::string p_node_name);

        // calculate transform matrix
        aiVector3D calcInterpolatedPosition(float p_animation_time, const aiNodeAnim* p_node_anim);
        aiQuaternion calcInterpolatedRotation(float p_animation_time, const aiNodeAnim* p_node_anim);
        aiVector3D calcInterpolatedScaling(float p_animation_time, const aiNodeAnim* p_node_anim);

        // To play multiple animations
        void readNodeHierarchy(float p_animation_time, const aiNode* p_node, const aiMatrix4x4 parent_transform);
        void boneTransform(double time_in_sec, std::vector<aiMatrix4x4>& transforms,int animationIndex);

        // To interpolate between two animations
        void readNodeHierarchyBlended(float p_start_time, float p_end_time, const aiNode* p_node,const aiMatrix4x4 parent_transform,int startAnimationIndex, int endAnimationIndex, float blendFactor);
        void boneTransformBlended(double time_in_sec,std::vector<aiMatrix4x4>& transforms,unsigned int StartAnimIndex,unsigned int EndAnimIndex,float BlendFactor);

    public:

        rvModel();
        ~rvModel();

        void ModelCleanUp();

        void initShaders(GLuint shader_program);
        void loadModel(const std::string& path);
        void loadAnimation(const std::string& path);

        // render
        void draw(GLuint shader_program,double dt);
        void drawInstanced(GLuint shader_program, double dt, GLint numOfInstances);

        void showNodeName(aiNode* node);

        aiQuaternion nlerp(aiQuaternion a, aiQuaternion b, float blend);
};