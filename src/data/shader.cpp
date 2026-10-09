#include "shader.hpp"
#include <iostream>
#include <fstream>
#include "resources.hpp"

using namespace Engine;

std::string ShaderProgram::getKey() {
    return m_key; 
}

std::shared_ptr<ShaderProgram> ShaderProgram::createProgramFromPath(std::filesystem::path shaderSource, std::string shaderKey) {
    
    if (shaderKey == UNNAMED_SHADER) {
        shaderKey = shaderSource.filename().string();
        size_t ind = shaderKey.find_last_of('.');
        shaderKey = shaderKey.substr(0, ind);
    }
    
    auto rcFile = ResourceManager::loadRaw(shaderSource.string()); 
    if (rcFile.size() > 0) {
        return createProgramFromData(rcFile.data(), shaderKey);
    }

    std::error_code ec;    
    if (!std::filesystem::exists(shaderSource, ec)) {
        std::cerr << "ShaderProgram file read error! -> " << ec.message() << std::endl;
        return nullptr;
    }

    auto _ = getShaderViaKey(shaderKey);
    if (_) return _; 

    std::ifstream file(shaderSource);
    std::stringstream buf;
    buf << file.rdbuf();
    file.close();

    return _createProgramFromStringStream(buf, shaderKey);

}

std::shared_ptr<ShaderProgram> ShaderProgram::createProgramFromData(unsigned char* data, std::string shaderKey) {
    
    auto _ = getShaderViaKey(shaderKey);
    if (_) return _;

    std::stringstream buf;
    buf << data;

    return _createProgramFromStringStream(buf, shaderKey);

}

std::shared_ptr<ShaderProgram> ShaderProgram::_createProgramFromStringStream(std::stringstream& ss, std::string shaderKey) {
    auto program = std::shared_ptr<ShaderProgram>(new ShaderProgram);
    program->m_key = shaderKey; 
    if (program->_compileProgram(ss) == SHADER_COMPILE_SUCCESS) {
        program->_registerProgram();

        return program;
    }

    return nullptr;
}

void ShaderProgram::useProgram() {
    if (GLContext::get().glState.m_currentProgram == m_programID || !m_registered) return;
    glUseProgram(m_programID);
}

void ShaderProgram::cleanup() {
    for (auto iterator = s_programCache.begin(); iterator != s_programCache.end();) {
        if (iterator->second.use_count() == 1) {
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

int ShaderProgram::_compileProgram(std::stringstream& shaderSource) {
    std::string vCode = "";
    std::string fCode = "";

    std::string line;
    std::string* writeBuf; 
    size_t index = 0;

    while (std::getline(shaderSource, line)) {
        
        if (line == shader_VertexIdentifier) {
            writeBuf = &vCode; 
            line = shader_vertexLayouts;
        }

        if (line == shader_FragmentIdentifier) {
            writeBuf = &fCode; 
            line = shader_fragmentLayouts;
        }

        if (line.find("main()") != std::string::npos && writeBuf == &vCode) {
            line += shader_vertexOut;
        }

        if (writeBuf == &fCode) {
            size_t index = line.find(shader_transpilerIdentifier);

            while (index != std::string::npos) {
                line.erase(line.begin() + index, line.begin() + index + 1);
                line.insert(index, "_transpilerID");
                index = line.find(shader_transpilerIdentifier, index + shader_transpilerIdentifier.length());
            }
        }

        if (writeBuf) {
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
        std::cerr << "ShaderProgram with key '" << m_key << "' has compilation errors!\nVertex Error(s):\n" << verror << "Fragment Error(s):\n" << ferror << std::endl;
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
        return iterator->second;
    }
    return nullptr;
}

int ShaderProgram::getUniformLoc(const std::string &uniformLoc) {
    if (m_uniformCache.find(uniformLoc) != m_uniformCache.end()) {
        return m_uniformCache[uniformLoc];
    }

    int loc = glGetUniformLocation(m_programID, uniformLoc.c_str());
    if (loc == -1) {
        std::cerr << "ShaderProgram with key '" << m_key << "' Uniform '" << uniformLoc << "' doesn't exist!\n";
    } 
    m_uniformCache.emplace(uniformLoc, loc);
    return loc;
}

void ShaderProgram::setUniform(const std::string& uniformLoc, float uniform) {
    glUniform1fv(getUniformLoc(uniformLoc), 1, &uniform);
}

void ShaderProgram::setUniform(const std::string& uniformLoc, int uniform) {
    glUniform1iv(getUniformLoc(uniformLoc), 1, &uniform);
}

void ShaderProgram::setUniform(const std::string& uniformLoc, bool uniform) {
    setUniform(uniformLoc, (int)uniform);
}

void ShaderProgram::setUniform(const std::string& uniformLoc, double uniform) {
    glUniform1dv(getUniformLoc(uniformLoc), 1, &uniform);
}

void ShaderProgram::setUniform(const std::string& uniformLoc, unsigned int uniform) {
    glUniform1uiv(getUniformLoc(uniformLoc), 1, &uniform);
}


void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_vec2<float>& uniform) {
    glUniform2fv(getUniformLoc(uniformLoc), 1, &uniform.x);
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_vec3<float>& uniform) {
    glUniform3fv(getUniformLoc(uniformLoc), 1, &uniform.x); 
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_vec4<float>& uniform) {
    glUniform4fv(getUniformLoc(uniformLoc), 1, &uniform.x); 
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_mat4x4<float>& uniform) {
    glUniformMatrix4fv(getUniformLoc(uniformLoc), 1, GL_FALSE, uniform.data()); 
}