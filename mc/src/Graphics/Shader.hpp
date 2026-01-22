// Shader.hpp
#pragma once
#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>

class Shader {
public:
    GLuint ID;
    Shader(const char* vertexSrc, const char* fragmentSrc);
    ~Shader();
    void use() const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
private:
    GLuint compile(GLenum type, const char* source);
};