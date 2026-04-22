#include "ResourceManager.h"
#include <iostream>
#include "stb_image.h" 

std::map<std::string, Shader*> ResourceManager::Shaders;
std::map<std::string, Model*> ResourceManager::Models;
std::map<std::string, unsigned int> ResourceManager::Textures;

Shader* ResourceManager::LoadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile) {
    Shader* shader = new Shader(vShaderFile, fShaderFile);
    Shaders[name] = shader;
    return shader;
}

Shader* ResourceManager::GetShader(const std::string& name) {
    return Shaders[name];
}

Model* ResourceManager::LoadModel(const std::string& name, const std::string& file) {
    stbi_set_flip_vertically_on_load(true); // 模型加载需要翻转Y
    Model* model = new Model(file);
    Models[name] = model;
    return model;
}

Model* ResourceManager::GetModel(const std::string& name) {
    return Models[name];
}

unsigned int ResourceManager::LoadTexture(const std::string& name, const char* file) {
    stbi_set_flip_vertically_on_load(false); // UI和普通纹理不翻转
    unsigned int textureID;
    glGenTextures(1, &textureID);
    
    int width, height, nrComponents;
    unsigned char *data = stbi_load(file, &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format = (nrComponents == 1) ? GL_RED : (nrComponents == 3) ? GL_RGB : GL_RGBA;
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << file << std::endl;
        stbi_image_free(data);
    }
    Textures[name] = textureID;
    return textureID;
}

unsigned int ResourceManager::GetTexture(const std::string& name) {
    return Textures[name];
}

unsigned int ResourceManager::LoadCubemap(const std::string& name, std::vector<std::string> faces) {
    stbi_set_flip_vertically_on_load(false);
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        } else {
            std::cout << "Cubemap failed to load: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    Textures[name] = textureID;
    return textureID;
}

void ResourceManager::Clear() {
    for (auto iter : Shaders) delete iter.second;
    for (auto iter : Models) delete iter.second;
}