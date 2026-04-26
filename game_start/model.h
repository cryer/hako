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

using namespace std;

unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);


class Model 
{
public:
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

    void Draw(Shader &shader)
    {
        for(unsigned int i = 0; i < meshes.size(); i++)
            meshes[i].Draw(shader);
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
        const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) 
        {
            cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
            return;
        }
   
        directory = path.substr(0, path.find_last_of('/'));
        processNode(scene->mRootNode, scene);


        // ========== 统计并打印顶点数量 ==========
        unsigned int totalVertices = 0;
        unsigned int totalIndices = 0;
        for(unsigned int i = 0; i < meshes.size(); i++)
        {
            totalVertices += meshes[i].vertices.size();
            totalIndices += meshes[i].indices.size();
        }
        cout << "Model loaded: " << path << endl;
        cout << "Total Meshes: " << meshes.size() << endl;
        cout << "Total Vertices: " << totalVertices << endl;
        cout << "Total Indices: " << totalIndices << endl;

        // 如果开启了计算AABB，可以在这里打印一下包围盒大小做验证
        // if (calculateAABB) {
        //     cout << "Model AABB Min: (" << localAABB.min.x << ", " << localAABB.min.y << ", " << localAABB.min.z << ")" << endl;
        //     cout << "Model AABB Max: (" << localAABB.max.x << ", " << localAABB.max.y << ", " << localAABB.max.z << ")" << endl;
        // }
        cout << "========================================" << endl;
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

        // 计算完成后，将当前网格的AABB合并进大模型总体的AABB中
        if (calculateAABB) {
            this->localAABB.Merge(meshAABB);
        }
        
        // 返回Mesh时带上 meshAABB
        return Mesh(vertices, indices, textures, meshAABB);
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
