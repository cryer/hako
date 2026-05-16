#pragma once

#include <glad/glad.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"
#include "AABB.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#define MAX_BONE_INFLUENCE 4

struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
    //bone indexes which will influence this vertex
    int m_BoneIDs[MAX_BONE_INFLUENCE];
    //weights from each bone
    float m_Weights[MAX_BONE_INFLUENCE];
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    // mesh Data
    std::vector<Vertex>       vertices;
    // 将索引数组扩充为 3 个（对应 LOD0, LOD1, LOD2）
    std::vector<unsigned int> indices[3];
    std::vector<Texture>      textures;
    unsigned int VAO;

    // 网格局部AABB
    AABB localAABB;

    // 构造函数，增加 AABB 参数
    Mesh(std::vector<Vertex> vertices, 
        std::vector<unsigned int> indicesLOD0, 
        std::vector<unsigned int> indicesLOD1, 
        std::vector<unsigned int> indicesLOD2,
        std::vector<Texture> textures,
        AABB aabb)
    {
        this->vertices = vertices;
        this->indices[0] = indicesLOD0;
        this->indices[1] = indicesLOD1;
        this->indices[2] = indicesLOD2;

        this->textures = textures;

        this->localAABB = aabb; // 赋值AABB

        setupMesh();
    }
    // Draw 函数接收 lodLevel，默认为 0
    void Draw(Shader &shader, int lodLevel = 0)
    {
        // 确保 lodLevel 不越界
        lodLevel = std::max(0, std::min(lodLevel, 2));

        unsigned int diffuseNr  = 1;
        unsigned int specularNr = 1;
        unsigned int normalNr   = 1;
        unsigned int heightNr   = 1;
        for(unsigned int i = 0; i < textures.size(); i++)
        {
            glActiveTexture(GL_TEXTURE0 + i);
            std::string number;
            std::string name = textures[i].type;
            if(name == "texture_diffuse")
                number = std::to_string(diffuseNr++);
            else if(name == "texture_specular")
                number = std::to_string(specularNr++);
            else if(name == "texture_normal")
                number = std::to_string(normalNr++);
             else if(name == "texture_height")
                number = std::to_string(heightNr++); 

            
            // glUniform1i(glGetUniformLocation(shader.ID, (name + number).c_str()), i);
            shader.setInt(name + number, i);
            glBindTexture(GL_TEXTURE_2D, textures[i].id);
        }
        
        // draw mesh
        glBindVertexArray(VAO);
        // 由于所有 LOD 共享顶点 VBO，只需要切换 EBO 即可无缝切换 LOD
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[lodLevel]);
        glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(indices[lodLevel].size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);


        // ===== 添加解绑操作，防止纹理污染 =====
        for(unsigned int i = 0; i < textures.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, 0); 
        }
        glActiveTexture(GL_TEXTURE0);
    }

private:
    // 我们需要 3 个 EBO
    unsigned int VBO, EBO[3];

    void setupMesh()
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(3, EBO); // 生成 3 个 EBO

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

        // 初始化时将 LOD 的 indices 存入各自的 EBO 中
        for(int i = 0; i < 3; i++) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[i]);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices[i].size() * sizeof(unsigned int), &indices[i][0], GL_STATIC_DRAW);
        }

        // （强制绑定回 EBO[0] 保证 VAO 默认记录 LOD0，虽然 Draw 会动态切换，这是好习惯）
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);


        // vertex Positions
        glEnableVertexAttribArray(0);   
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        // vertex normals
        glEnableVertexAttribArray(1);   
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
        // vertex texture coords
        glEnableVertexAttribArray(2);   
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
        // vertex tangent
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
        // vertex bitangent
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));
        // ids
        glEnableVertexAttribArray(5);
        glVertexAttribIPointer(5, 4, GL_INT, sizeof(Vertex), (void*)offsetof(Vertex, m_BoneIDs));

        // weights
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_Weights));
        glBindVertexArray(0);
    }
};
