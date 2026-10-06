#include "shader.hpp"
#include <iostream>
#include <fstream>


using namespace Engine;

std::shared_ptr<ShaderProgram> ShaderProgram::createProgram(std::filesystem::path shaderSource, std::string shaderKey) {
    
    std::error_code ec;    
    if (!std::filesystem::exists(shaderSource, ec)) {
        std::cerr << "ShaderProgram file read error! -> " << ec.message() << std::endl;
        return nullptr;
    }

    if (shaderKey == UNNAMED_SHADER) {
        shaderKey = shaderSource.filename().string();
        size_t ind = shaderKey.find_last_of('.');
        shaderKey = shaderKey.substr(0, ind);
    }

    auto _ = getShaderViaKey(shaderKey);

    if (_) return _; 

    auto program = std::make_shared<ShaderProgram>();
    program->m_key = shaderKey; 
    if (program->_compileProgram(shaderSource) == SHADER_COMPILE_SUCCESS) {
        program->_registerProgram();
    }

    return nullptr;

}

void ShaderProgram::useProgram() {
    if (GLContext::get().glState.m_currentProgram == m_programID) return;
    glUseProgram(m_programID);
}

void ShaderProgram::cleanup() {
    for (auto iterator = s_programCache.begin(); iterator != s_programCache.end();) {
        if (iterator->second.expired()) {
            iterator = s_programCache.erase(iterator);
        }
        else {
            iterator++;
        }
    }
}

void ShaderProgram::_registerProgram() {
    s_programCache.emplace(m_key, shared_from_this());
}

void ShaderProgram::_unregisterProgram() {
    if (getShaderViaKey(m_key)) s_programCache.erase(m_key);
}

int ShaderProgram::_compileProgram(std::filesystem::path shaderSource) {
    std::string vCode = "";
    std::string fCode = "";

    std::ifstream file(shaderSource);

    if (!file) {
        std::cerr << "Shader file unable to be opened." << std::endl;
        return ERR_SHADER_FILE_NOT_FOUND;
    }

    std::string line;
    std::string* writeBuf; 
    size_t index = 0;

    while (std::getline(file, line)) {
        
        if (line == shader_VertexIdentifier) {
            writeBuf = &vCode; 
            line = shader_GLVersion;
        }

        if (line == shader_FragmentIdentifier) {
            writeBuf = &fCode; 
            line = shader_GLVersion;
        }

        if (writeBuf && index != 0) {
            *writeBuf += line + "\n";
        }

        index++;
    }

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

    const char* vCode_cstr = vCode.c_str();
    const char* fCode_cstr = fCode.c_str();

    glShaderSource(vert, 1, &vCode_cstr, NULL);
    glShaderSource(frag, 1, &fCode_cstr, NULL);

    glCompileShader(vert);
    glCompileShader(frag);

    const char* verror = _getShaderCompileErrors(vert);
    const char* ferror = _getShaderCompileErrors(frag);
    if (verror || ferror) {
        std::cerr << "Shader dump:\nVertex:\n\n" << vCode << "\n\nFragment:\n\n" << fCode << std::endl; 
        std::cerr << "ShaderProgram compilation errors!\nVertex Error(s):\n" << verror << "Fragment Error(s):\n" << ferror << std::endl;
        return ERR_SHADER_COMPILATION_FAILED;
    }

    m_programID = glCreateProgram();
    glAttachShader(m_programID, vert);
    glAttachShader(m_programID, frag);
    glLinkProgram(m_programID);

    glDeleteShader(vert);
    glDeleteShader(frag);

    return SHADER_COMPILE_SUCCESS;
}

const char* ShaderProgram::_getShaderCompileErrors(GLuint shader) {
    int success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        const char* _ = log;
        return _; 
    }
    return NULL;
}

std::shared_ptr<ShaderProgram> ShaderProgram::getShaderViaKey(std::string shaderKey) {
    auto iterator = s_programCache.find(shaderKey);
    if (iterator != s_programCache.end()) {
        auto shader = iterator->second.lock();
        return shader;
    }
    std::cerr << "ShaderProgram with key " << shaderKey << " not found." << std::endl;
    return nullptr;
}