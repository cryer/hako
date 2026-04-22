#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>


class Shader
{
public:
    unsigned int ID;
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char* vertexPath, const char* fragmentPath)
    {

        // 👇 提取纯文件名（去掉路径）
        auto getFileName = [](const char* path) -> std::string {
            std::string s(path);
            size_t pos = s.find_last_of("/\\");
            return (pos == std::string::npos) ? s : s.substr(pos + 1);
        };
        
        std::string vertexName = getFileName(vertexPath);
        std::string fragmentName = getFileName(fragmentPath);

        // 1. retrieve the vertex/fragment source code from filePath
        std::string vertexCode;
        std::string fragmentCode;
        std::ifstream vShaderFile;
        std::ifstream fShaderFile;
        // ensure ifstream objects can throw exceptions:
        vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
        try 
        {
            // open files
            vShaderFile.open(vertexPath);
            fShaderFile.open(fragmentPath);
            std::stringstream vShaderStream, fShaderStream;
            // read file's buffer contents into streams
            // rdbuf()返回指向原始缓冲区streambuf的指针，需要借助流来读取
            vShaderStream << vShaderFile.rdbuf();
            fShaderStream << fShaderFile.rdbuf();
            // close file handlers
            vShaderFile.close();
            fShaderFile.close();
            // convert stream into string
            vertexCode   = vShaderStream.str();
            fragmentCode = fShaderStream.str();
        }
        catch (std::ifstream::failure& e)
        {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        }
        /*
        更优雅，更简洁的符合STL风格的写法，利用[beg，end）迭代器逐个拷贝字符
        同样绕过 格式化层。和上面原理不同，上面是直接复制底层fstream原始缓冲区
        到stringstream缓冲区，然后str()获取string数据
        两者都读取所有字符，绕过格式化，性能相差不大，下面更简洁，上面更直观
        std::istreambuf_iterator<char>()
        其实就是默认内部指向streambuf的指针是nullptr，而在文件中就可以表示文件结束
        而begin自然就是内部指向streambuf的指针指向file文件流底层的streambuf
        （streambuf是原始缓冲区，分为i和o输入输出，往上抽象成filebuf，stringbuf等。
        stream流内部包含buf缓冲，buf存储真正的数据，但本身是数据管理器而非单纯数据容器，流则进一步是buf的管理器，比如可以格式化输入输出等。
        streambuf中除了stringbuf特殊外【直接str()获取】，其他buf中的数据不可以直接访问，都是protect属性，这是为了保证流的同步，buf如果通过指针直接读取了，破坏了状态，比如指针位置等，而buf的管理器也就是流是未知的。所以像rdbuf这样返回streambuf，其实也就是filebuf的指针，不能直接获取数据，而是要通过流操作获取，可以流入到任意的其他stream中，比如cout的ostream，ostringstream或者流入另一个ofstream。）
        【其实都是直接操作底层缓冲区streambuf】
        std::ifstream vShaderFile(vertexPath);
        std::string vertexCode(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
        );
        */
        const char* vShaderCode = vertexCode.c_str();
        const char * fShaderCode = fragmentCode.c_str();
        // 2. compile shaders
        unsigned int vertex, fragment;
        // vertex shader
        vertex = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertex, 1, &vShaderCode, NULL);
        glCompileShader(vertex);
        checkCompileErrors(vertex, "VERTEX", vertexName);
        // fragment Shader
        fragment = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragment, 1, &fShaderCode, NULL);
        glCompileShader(fragment);
        checkCompileErrors(fragment, "FRAGMENT", fragmentName);
        // shader Program
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        checkCompileErrors(ID, "PROGRAM", vertexName + " + " + fragmentName);
        // delete the shaders as they're linked into our program now and no longer necessary
        glDeleteShader(vertex);
        glDeleteShader(fragment);
    }
    // activate the shader
    // ------------------------------------------------------------------------
    void use() 
    { 
        glUseProgram(ID); 
    }
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const
    {         
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
    }
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const
    { 
        glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
    }
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const
    { 
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
    }

    void setFloat2(const std::string &name, float value1, float value2) const
    { 
        glUniform2f(glGetUniformLocation(ID, name.c_str()), value1,
            value2); 
    }

    void setFloat3(const std::string &name, float value1, float value2,
        float value3) const
    { 
        glUniform3f(glGetUniformLocation(ID, name.c_str()), value1,
            value2, value3); 
    }

    void setFloat3(const std::string &name, const glm::vec3 &vec) const
    { 
        glUniform3fv(glGetUniformLocation(ID, name.c_str()), 
            1, &vec[0]); 
    }

    void setFloat4(const std::string &name, float value1, float value2,float value3, float value4 ) const
    { 
        glUniform4f(glGetUniformLocation(ID, name.c_str()), value1, value2,value3, value4); 
    }

    void setFloat4(const std::string &name, const glm::vec4 &vec) const
    { 
        glUniform4fv(glGetUniformLocation(ID, name.c_str()), 
            1, &vec[0]); 
    }

    void setMatrix4fv(const std::string &name, const glm::mat4 &mat) const
    { 
        glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 
            1, GL_FALSE,
            glm::value_ptr(mat)); 
    }

private:
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    void checkCompileErrors(unsigned int shader, std::string type, std::string name)
    {
        int success;
        char infoLog[1024];
        if (type != "PROGRAM")
        {
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if (!success)
            {
                glGetShaderInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
                // 👈 添加 shader 名称
                std::cout << " | Shader: " << name << std::endl;  
            }
        }
        else
        {
            glGetProgramiv(shader, GL_LINK_STATUS, &success);
            if (!success)
            {
                glGetProgramInfoLog(shader, 1024, NULL, infoLog);
                std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
            }
        }
    }
};
#endif