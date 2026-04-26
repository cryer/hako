#pragma once
#include <map>
#include <string>
#include <vector>
#include <glad/glad.h>
#include "shader.h"
#include "model.h"


class ResourceManager {
public:
    static std::map<std::string, Shader*> Shaders;
    static std::map<std::string, Model*> Models;
    static std::map<std::string, unsigned int> Textures;

    static Shader* LoadShader(const std::string& name, const char* vShaderFile, const char* fShaderFile);
    static Shader* GetShader(const std::string& name);

    static Model* LoadModel(const std::string& name, const std::string& file, bool needAABB = true);
    static Model* GetModel(const std::string& name);

    static unsigned int LoadTexture(const std::string& name, const char* file);
    static unsigned int GetTexture(const std::string& name);

    static unsigned int LoadCubemap(const std::string& name, std::vector<std::string> faces);

    static void Clear();
};