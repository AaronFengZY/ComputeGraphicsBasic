/*
 * Shader.h
 * 
 * ??????????????OpenGL????????????????????????????????????
 * ??????????????????????????uniform????????
 * 
 * ?????
 * - ?????????
 * - ?????????????
 * - ??????????
 * - ??uniform?????????bool?vec3?mat4???
 * - ???????????
 * 
 * ????Zhiyuan Feng
 */

#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h" // ??glad????????OpenGL???
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
        // ??ID
        unsigned int programID;

        // ???????????
        Shader(const GLchar* vertexFilePath, const GLchar* fragmentFilePath, const char* geometryFilePath = nullptr) {
            // ?????????/???????
            std::string vertexSource = loadShaderSource(vertexFilePath);
            std::string fragmentSource = loadShaderSource(fragmentFilePath);
            std::string geometrySource = geometryFilePath ? loadShaderSource(geometryFilePath) : "";

            // ?????
            unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource.c_str());
            unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource.c_str());
            unsigned int geometryShader = 0;

            if (!geometrySource.empty()) {
                geometryShader = compileShader(GL_GEOMETRY_SHADER, geometrySource.c_str());
            }

            // ???????
            programID = glCreateProgram();
            glAttachShader(programID, vertexShader);
            glAttachShader(programID, fragmentShader);
            if (geometryShader) glAttachShader(programID, geometryShader);
            glLinkProgram(programID);

            // ??????
            checkCompileErrors(programID, "PROGRAM");

            // ???????????????????????
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

        void setBool(const std::string &name, bool value) const {
            glUniform1i(glGetUniformLocation(programID, name.c_str()), (int)value);
        }
        void setInt(const std::string &name, int value) const {
            glUniform1i(glGetUniformLocation(programID, name.c_str()), value); 
        }
        void setFloat(const std::string &name, float value) const {
            glUniform1f(glGetUniformLocation(programID, name.c_str()), value); 
        }
        // ------------------------------------------------------------------------
        void setVec2(const std::string &name, const glm::vec2 &value) const{ 
            glUniform2fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]); 
        }
        void setVec2(const std::string &name, float x, float y) const{ 
            glUniform2f(glGetUniformLocation(programID, name.c_str()), x, y); 
        }
        void setVec3(const std::string &name, const glm::vec3 &value) const{ 
            glUniform3fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]); 
        }
        void setVec3(const std::string &name, float x, float y, float z) const{ 
            glUniform3f(glGetUniformLocation(programID, name.c_str()), x, y, z); 
        }
        void setVec4(const std::string &name, const glm::vec4 &value) const{ 
            glUniform4fv(glGetUniformLocation(programID, name.c_str()), 1, &value[0]); 
        }
        void setVec4(const std::string &name, float x, float y, float z, float w) { 
            glUniform4f(glGetUniformLocation(programID, name.c_str()), x, y, z, w); 
        }
        // ------------------------------------------------------------------------
        void setMat2(const std::string &name, const glm::mat2 &mat) const{
            glUniformMatrix2fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        void setMat3(const std::string &name, const glm::mat3 &mat) const{
            glUniformMatrix3fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }
        void setMat4(const std::string &name, const glm::mat4 &mat) const{
            glUniformMatrix4fv(glGetUniformLocation(programID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
        }

        // ???????
        void disableShaders() const {
            glUseProgram(0); // ?????
        }
};

#endif




