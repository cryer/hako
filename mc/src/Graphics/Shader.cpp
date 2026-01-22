// Shader.cpp
#include "Shader.hpp"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

Shader::Shader(const char* vSrc, const char* fSrc) {
    GLuint v = compile(GL_VERTEX_SHADER, vSrc);
    GLuint f = compile(GL_FRAGMENT_SHADER, fSrc);
    ID = glCreateProgram();
    glAttachShader(ID, v);
    glAttachShader(ID, f);
    glLinkProgram(ID);
    glDeleteShader(v);
    glDeleteShader(f);
}

Shader::~Shader() { glDeleteProgram(ID); }
void Shader::use() const { glUseProgram(ID); }
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

GLuint Shader::compile(GLenum type, const char* src) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, NULL);
    glCompileShader(s);
    return s;
}