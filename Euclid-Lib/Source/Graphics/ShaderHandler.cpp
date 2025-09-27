#include "Graphics.hpp"

namespace Euclid
{
void Shader::Init(GLenum shaderType, const char* shaderCode) {
    mShaderID = glCreateShader(shaderType);
    
    glShaderSource(mShaderID, 1, &shaderCode, NULL);
    glCompileShader(mShaderID);
    
    CheckCompileErrors();
}
unsigned int Shader::GetID() {
    return mShaderID;
}
Shader::~Shader() {
    glDeleteShader(mShaderID);
}
void Shader::CheckCompileErrors() {
    int success;
    char infoLog[1024];
    glGetShaderiv(mShaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(mShaderID, 1024, NULL, infoLog);
        std::cout << "ERROR::SHADER_COMPILATION_ERROR" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}
}

namespace Euclid
{
void ShaderProgram::Init(std::span<unsigned int> shaderIDs) {
    mProgramID = glCreateProgram();
    for (auto shaderID : shaderIDs) glAttachShader(mProgramID, shaderID);
    glLinkProgram(mProgramID);
    CheckCompileErrors();
}
unsigned int ShaderProgram::GetID() {
    return mProgramID;
}
void ShaderProgram::Use() {
    glUseProgram(mProgramID);
}
ShaderProgram::~ShaderProgram() {
    glDeleteProgram(mProgramID);
}
void ShaderProgram::CheckCompileErrors() {
    int success;
    char infoLog[1024];
    glGetProgramiv(mProgramID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(mProgramID, 1024, NULL, infoLog);
        std::cout << "ERROR::PROGRAM_LINKING_ERROR" << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
    }
}
}

