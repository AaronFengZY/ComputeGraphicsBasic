/*
 * Shader.h
 * 
 * 这是一个用于加载、编译、链接OpenGL着色器的工具类。该类支持加载顶点着色器、片段着色器和几何着色器（可选）。
 * 它提供了编译和链接错误检查、着色器程序的管理以及设置uniform变量的工具函数。
 * 
 * 主要功能：
 * - 加载着色器源码文件
 * - 编译顶点、片段、几何着色器
 * - 创建和链接着色器程序
 * - 提供uniform变量设置函数（包括bool、vec3、mat4类型）
 * - 支持禁用当前着色器程序
 * 
 * 撰写者：Zhiyuan Feng
 */

#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h" // 包含glad来获取所有的必须OpenGL头文件
#include "glm/glm.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

// Helper function to check for compilation and linking errors
void checkCompileErrors(unsigned int shader, const std::string &type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, infoLog);
            std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
    }
}

// Helper function to load shader source from file
std::string loadShaderSource(const std::string &filePath) {
    std::ifstream shaderFile;
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        return shaderStream.str();
    } catch (std::ifstream::failure &e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath << std::endl;
        return "";
    }
}

// Helper function to compile shader and check for errors
unsigned int compileShader(GLenum shaderType, const char *shaderCode) {
    unsigned int shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderCode, nullptr);
    glCompileShader(shader);
    checkCompileErrors(shader, (shaderType == GL_VERTEX_SHADER ? "VERTEX" :
                                shaderType == GL_FRAGMENT_SHADER ? "FRAGMENT" : "GEOMETRY"));
    return shader;
}



class Shader
{
    public:
        // 程序ID
        unsigned int programID;

        // 构造器读取并构建着色器
        Shader(const GLchar* vertexFilePath, const GLchar* fragmentFilePath, const char* geometryFilePath = nullptr) {
            // 从文件路径获取顶点/片段着色器源码
            std::string vertexSource = loadShaderSource(vertexFilePath);
            std::string fragmentSource = loadShaderSource(fragmentFilePath);
            std::string geometrySource = geometryFilePath ? loadShaderSource(geometryFilePath) : "";

            // 编译着色器
            unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.c_str());
            unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
            unsigned int geometryShader = 0;

            if (!geometrySource.empty()) {
                geometryShader = compileShader(GL_GEOMETRY_SHADER, geometrySource.c_str());
            }

            // 创建着色器程序
            programID = glCreateProgram();
            glAttachShader(programID, vertexShader);
            glAttachShader(programID, fragmentShader);
            if (geometryShader) glAttachShader(programID, geometryShader);
            glLinkProgram(programID);

            // 检查链接错误
            checkCompileErrors(programID, "PROGRAM");

            // 删除单独的着色器对象（它们已经被链接到程序中）
            glDeleteShader(vertexShader);
            glDeleteShader(fragmentShader);
            if (geometryShader) glDeleteShader(geometryShader);
        }

        ~Shader() {
            glDeleteProgram(programID);
        }

        void use() {
            glUseProgram(programID);
        }

        // uniform工具函数
        void setBool(const std::string &name, bool value) const {
            glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
        }

        void setVec3(const std::string &name, const glm::vec3 &value) const { 
            glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]); 
        }

        void setMat4(const std::string &name, const glm::mat4 &mat) const {
            glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        // 禁用当前着色器
        void disableShaders() const {
            glUseProgram(0); // 禁用着色器
        }
};

#endif
