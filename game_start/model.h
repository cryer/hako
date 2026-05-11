#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// 不要在头文件中定义stb_image.h的实现
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.h"
#include "shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <meshoptimizer.h>

using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);


class Model 
{
public:
    // LOD 及统计数据
    string modelName;
    int currentLOD = -1; // 记录当前 LOD 以便比对
    unsigned int lodVertexCount[3] = {0, 0, 0};
    unsigned int lodFaceCount[3]   = {0, 0, 0};

    // model data 
    vector<Texture> textures_loaded;
    vector<Mesh>    meshes;
    string directory;
    bool gammaCorrection;

    AABB localAABB;
    bool calculateAABB;


    Model(string const &path,
        bool calcAABB = true,
        bool gamma = false) : 
        gammaCorrection(gamma),
        calculateAABB(calcAABB){
        loadModel(path);
    }
    // Draw 函数接收并向下传递 lodLevel
    void Draw(Shader &shader, int lodLevel = 0)
    {
        lodLevel = std::max(0, std::min(lodLevel, 2));

        if (lodLevel != currentLOD) {
            currentLOD = lodLevel;
        }

        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader, currentLOD);
    }

    // 强制给模型的所有 Mesh 设置一张漫反射贴图（无视原有的 mtl 设置）
    void SetDiffuseTexture(const string& textureFilename)
    {
        // 1. 加载这张图片作为纹理
        Texture tex;
        tex.id = TextureFromFile(textureFilename.c_str(), this->directory);
        tex.type = "texture_diffuse";
        tex.path = textureFilename;

        // 2. 遍历模型里的所有网格(Mesh)
        for(unsigned int i = 0; i < meshes.size(); i++)
        {
            // 清理掉可能存在的旧的 diffuse 贴图，保留 normal 或 specular 等其他贴图
            vector<Texture> newTextures;
            for(unsigned int j = 0; j < meshes[i].textures.size(); j++)
            {
                if(meshes[i].textures[j].type != "texture_diffuse")
                {
                    newTextures.push_back(meshes[i].textures[j]);
                }
            }
            
            // 3. 把我们指定的贴图塞进去
            newTextures.push_back(tex);
            meshes[i].textures = newTextures;
        }
        
        cout << "Forced texture " << textureFilename << " applied to model." << endl;
    }
    
private:
    void loadModel(string const &path)
    {
        Assimp::Importer importer;
        // 这里必须加上 aiProcess_JoinIdenticalVertices ！！！
        // 否则所有的面都是断开的，不仅无法生成 LOD，还会导致极高的内存占用和极差的渲染性能。
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_JoinIdenticalVertices);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }

        // 提取模型名
        size_t lastSlash = path.find_last_of('/');
        modelName = (lastSlash == string::npos) ? path : path.substr(lastSlash + 1);
   
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);


        // 精确统计每个 LOD 级别中，实际使用的“唯一顶点数”和“面数”
        for (int lod = 0; lod < 3; lod++) {
            lodFaceCount[lod] = 0;
            lodVertexCount[lod] = 0;
            for (auto& mesh : meshes) {
                lodFaceCount[lod] += mesh.indices[lod].size() / 3;
                
                // 计算该 LOD 降级后，实际真正还在被引用的顶点数量
                vector<bool> used(mesh.vertices.size(), false);
                unsigned int uniqueVerts = 0;
                for (unsigned int idx : mesh.indices[lod]) {
                    if (!used[idx]) { used[idx] = true; uniqueVerts++; }
                }
                lodVertexCount[lod] += uniqueVerts;
            }
        }

        // 打印模型加载完毕的全局信息
        cout << "Model loaded: " << modelName << endl;
        cout << "actual rendering statistic :" << endl;
        for (int i = 0; i < 3; i++) {
            cout << "  - LOD" << i << " | faces: " << lodFaceCount[i] << ", vertex: " << lodVertexCount[i] << endl;
        }
        cout << "========================================" << endl;

        // ========== 统计并打印顶点数量 ==========
        // unsigned int totalVertices = 0;
        // unsigned int totalIndices = 0;
        // for(unsigned int i = 0; i < meshes.size(); i++)
        // {
        //     totalVertices += meshes[i].vertices.size();
        //     totalIndices += meshes[i].indices.size();
        // }
        // cout << "Model loaded: " << path << endl;
        // cout << "Total Meshes: " << meshes.size() << endl;
        // cout << "Total Vertices: " << totalVertices << endl;
        // cout << "Total Indices: " << totalIndices << endl;
        // cout << "========================================" << endl;
    }

    void processNode(aiNode *node, const aiScene *scene)
    {
        for(unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            meshes.push_back(processMesh(mesh, scene));
        }
        for(unsigned int i = 0; i < node->mNumChildren; i++)
        {
            processNode(node->mChildren[i], scene);
        }

    }

    Mesh processMesh(aiMesh *mesh, const aiScene *scene)
    {
        vector<Vertex> vertices;
        vector<unsigned int> indices;
        vector<Texture> textures;

        // 当前Mesh的局部AABB
        AABB meshAABB;

        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;

            glm::vec3 vector; 
            // positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;

            // 在获取顶点后，立刻扩充AABB (且几乎没有性能损耗)
            if (calculateAABB) {
                meshAABB.Expand(vector);
            }

            // normals
            if (mesh->HasNormals())
            {
                vector.x = mesh->mNormals[i].x;
                vector.y = mesh->mNormals[i].y;
                vector.z = mesh->mNormals[i].z;
                vertex.Normal = vector;
            }
            // texture coordinates
            if(mesh->mTextureCoords[0]) 
            {
                glm::vec2 vec;
   
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                // tangent
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
                // bitangent
                vector.x = mesh->mBitangents[i].x;
                vector.y = mesh->mBitangents[i].y;
                vector.z = mesh->mBitangents[i].z;
                vertex.Bitangent = vector;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);

            vertices.push_back(vertex);
        }
        // 读取face
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for(unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);        
        }
        //读取 Material
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];    
        // diffuse: texture_diffuseN
        // specular: texture_specularN
        // normal: texture_normalN

        // 1. diffuse maps
        vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        // 2. specular maps
        vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
        textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());
      
        //如果没有贴图，就传颜色给 Shader，针对没有图片的场景
        if (diffuseMaps.empty()) {
            aiColor3D color(0.8f, 0.8f, 0.8f); // 默认灰色
            material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
            
            // 生成一张 1x1 的纯色贴图
            unsigned int colorTexture;
            glGenTextures(1, &colorTexture);
            glBindTexture(GL_TEXTURE_2D, colorTexture);
            unsigned char pixel[3] = { (unsigned char)(color.r * 255), (unsigned char)(color.g * 255), (unsigned char)(color.b * 255) };
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, pixel);
            
            Texture tex;
            tex.id = colorTexture;
            tex.type = "texture_diffuse";
            tex.path = "auto_generated_color_" + std::to_string(color.r) + std::to_string(color.g); // 随便给个唯一名字
            textures.push_back(tex);
        }

        // ================= MeshOptimizer 核心优化逻辑 =================
        // 1. 【GPU顶点缓存优化】重排 LOD0 索引，提升 Vertex Shader 执行效率
        meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertices.size());

        // 2. 【显存带宽读取优化】重排 LOD0 的顶点物理存储顺序，大幅提升内存局部性
        // 此函数会同时修改 vertices 和 indices 数组。
        meshopt_optimizeVertexFetch(vertices.data(), indices.data(), indices.size(), 
                                    vertices.data(), vertices.size(), sizeof(Vertex));

        // 3. 【生成 LOD1 - 50%面数】
        size_t target_indices_lod1 = size_t(indices.size() * 0.5f);
        vector<unsigned int> indicesLOD1(indices.size());
        size_t lod1_size = meshopt_simplify(
            indicesLOD1.data(), 
            indices.data(), indices.size(), // 以 LOD0 为基础简化
            &vertices[0].Position.x, vertices.size(), sizeof(Vertex),
            target_indices_lod1, 0.10f // 误差可调节
        );

        // 【保底】：如果因为 UV 缝隙等硬边界导致无法精简到目标面数
        // 则改用 Sloppy 算法强制无视拓扑缝隙精简到 50%
        // 【仅作保留】 如果强制使用，会导致很多模型，尤其是顶点少的模型失去基本外形
        // if (lod1_size > target_indices_lod1 * 1.1f) { 
        //     lod1_size = meshopt_simplifySloppy(
        //         indicesLOD1.data(), 
        //         indices.data(), indices.size(),
        //         &vertices[0].Position.x, vertices.size(), sizeof(Vertex),
        //         target_indices_lod1, 0.5f, nullptr
        //     );
        // }

        indicesLOD1.resize(lod1_size);
        // 对生成的 LOD1 也做一次顶点缓存优化
        meshopt_optimizeVertexCache(indicesLOD1.data(), indicesLOD1.data(), indicesLOD1.size(), vertices.size());

        // 4. 【生成 LOD2 - 25%面数】
        size_t target_indices_lod2 = size_t(indices.size() * 0.25f);
        vector<unsigned int> indicesLOD2(indices.size());
        size_t lod2_size = meshopt_simplify(
            indicesLOD2.data(), 
            indicesLOD1.data(), indicesLOD1.size(), // 阶梯式：以 LOD1 为基础简化，更快
            &vertices[0].Position.x, vertices.size(), sizeof(Vertex),
            target_indices_lod2, 0.20f  // 误差
        );

        // 【保底】
        // 【仅作保留】 如果强制使用，会导致很多模型，尤其是顶点少的模型失去基本外形
        // if (lod2_size > target_indices_lod2 * 1.1f) {
        //     lod2_size = meshopt_simplifySloppy(
        //         indicesLOD2.data(), 
        //         indicesLOD1.data(), indicesLOD1.size(),
        //         &vertices[0].Position.x, vertices.size(), sizeof(Vertex),
        //         target_indices_lod2, 0.5f, nullptr
        //     );
        // }

        indicesLOD2.resize(lod2_size);
        // 对生成的 LOD2 做一次顶点缓存优化
        meshopt_optimizeVertexCache(indicesLOD2.data(), indicesLOD2.data(), indicesLOD2.size(), vertices.size());

        // =======================================================================

        // 计算完成后，将当前网格的AABB合并进大模型总体的AABB中
        if (calculateAABB) {
            this->localAABB.Merge(meshAABB);
        }
        
        // 返回Mesh时带上 meshAABB ,传入三个级别的索引
        return Mesh(vertices, indices, indicesLOD1, indicesLOD2, textures, meshAABB);
    }

    
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
    {
        vector<Texture> textures;
        for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
        {
            aiString str;
            mat->GetTexture(type, i, &str);
            bool skip = false;
            for(unsigned int j = 0; j < textures_loaded.size(); j++)
            {
                if(std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
                {
                    textures.push_back(textures_loaded[j]);
                    skip = true; 
                    break;
                }
            }
            if(!skip)
            {   
                Texture texture;
                texture.id = TextureFromFile(str.C_Str(), this->directory);
                texture.type = typeName;
                texture.path = str.C_Str();
                textures.push_back(texture);
                textures_loaded.push_back(texture);
            }
        }
        return textures;
    }
};
