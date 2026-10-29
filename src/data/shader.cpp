#include "logger.hpp"
#include <shader.hpp>
#include <iostream>
#include <fstream>
#include <resources.hpp>
#include <engine.h>
#include <material.hpp>

using namespace Engine;

std::unordered_map<std::string, CacheID>& ShaderProgram::stringToIDMap() {
    static std::unordered_map<std::string, CacheID> map;
    return map;
}

ShaderProgram::~ShaderProgram() {

}

void ShaderProgram::_createDefaultShaders() {
    // lit shader
    std::shared_ptr<ShaderProgram> lit = createProgramFromPath("stuff=>shaders/shader_lit.glsl", shader_litShader);
    // mark this shader as don't delete to avoid cleanup.
    // we could just mark these as static but we could also do this.
    // and this is cooler i think. 
    lit->m_dontDelete = true;

    std::shared_ptr<ShaderProgram> col = createProgramFromPath("stuff=>shaders/collider.glsl", shader_colliderShader);
    col->m_dontDelete = true;

    std::shared_ptr<ShaderProgram> line = createProgramFromPath("stuff=>shaders/line.glsl", shader_lineShader);
    line->m_dontDelete = true;
    // put any other permanent shaders here
}

// returns this shader's key (or name)
std::string ShaderProgram::getKey() {
    return m_key; 
}
// create program from a file on the disk.
// can also be used to get a program from an archive
// on the disk with the help of the resource manager.
// the shader source is the path to the shader,
// the shader key is the name to use to find the shader
// in the shader cache.
std::shared_ptr<ShaderProgram> ShaderProgram::createProgramFromPath(std::filesystem::path shaderSource, std::string shaderKey) {
    
    // if shaderKey is unassigned, generate one from the filename.
    if (shaderKey == UNNAMED_SHADER) {
        // get the filename string
        shaderKey = shaderSource.filename().string();

        // remove the extension from the filename
        size_t slash = shaderKey.find_last_of('/');
        size_t dot   = shaderKey.find_last_of('.');
        shaderKey = shaderKey.substr(slash + 1, dot - slash - 1);
    }
    
    // try get an existing shader using the key.
    // if one is found, return that. otherwise continue.
    auto _ = getShaderViaKey(shaderKey);
    if (_) return _; 
    // no cached shader found, continue.
    
    // attempt to read an archived file
    bytes rcFile = ResourceManager::loadRaw(shaderSource.string()); 
    if (rcFile.size() > 0) {
        // file was in an archive, load it as data instead.
        return createProgramFromData(rcFile.data(), rcFile.size(), shaderKey);
    }
    // file was not in any archives in the string, continue loading 
    // as a regular file on disk.

    // try read the file
    std::error_code ec;    
    if (!std::filesystem::exists(shaderSource, ec)) {
        // file read failed, print error message.
        log().logError("ShaderProgram file read error! -> {}", ec.message());
        return nullptr;
    }
    // file read success!

    // open the file
    std::ifstream file(shaderSource);

    // read file contents into a buffer
    std::stringstream buf;
    buf << file.rdbuf();

    // close file. data is in the buffer now.
    file.close();

    // create a program from the string stream data.
    return _createProgramFromStringStream(buf, shaderKey);

}

// create program from an unsigned char array of data.
// to make it clear, an unsigned char array is the raw
// file data loaded into memory. 
// aside from that, functionally the same as the above function.
std::shared_ptr<ShaderProgram> ShaderProgram::createProgramFromData(unsigned char* data, size_t shaderSize, std::string shaderKey) {
    
    // try get an existing shader in the cache
    auto _ = getShaderViaKey(shaderKey);
    if (_) return _;
    // no cached shader, continue

    // write the unsigned char buf to a stringstream buf.
    std::stringstream buf;
    buf.write(reinterpret_cast<const char*>(data), shaderSize);

    // pass the stringstream to get compiled.
    return _createProgramFromStringStream(buf, shaderKey);

}

// create program from a string stream. this string stream gets passed right through to _compileProgram().
std::shared_ptr<ShaderProgram> ShaderProgram::_createProgramFromStringStream(std::stringstream& ss, std::string shaderKey) {
    // create a new shader program
    auto program = std::shared_ptr<ShaderProgram>(new ShaderProgram);
    
    // set the new program's key
    program->m_key = shaderKey;

    // try compile the program.
    if (program->_compileProgram(ss) == SHADER_COMPILE_SUCCESS) {
        // if compilation succeeded, add the shader to the cache.
        program->_register();
        stringToIDMap().emplace(shaderKey, program->getCacheID());

        return program;
    }
    // compilation failed.

    return nullptr;
}

// binds this shader program to the gl state if it is not already bound;
// marks this program as the shader to use for drawing.
void ShaderProgram::useProgram() {

    // if this program is already bound or was not registered, return early.
    if (GLContext::get().glState.m_currentProgram == m_programID) return;

    // bind the program.
    glUseProgram(m_programID);
}

// compile the program via stringstream.
int ShaderProgram::_compileProgram(std::stringstream& shaderSource) {

    // So what is happening here is we are separating the fragment
    // and vertex code out from the custom shading language,
    // and transpiling it to actual compilable glsl code.
    // Think of it as neatly unravelling a piece of scrunched up paper.
    // this is really difficult to explain, ask me directly if you don't
    // understand anything

    // create our buffers for the individual vertex and fragment code
    std::string vCode = "";
    std::string fCode = "";

    // create our buffer for the current line of the shader source.
    std::string line;

    // writeBuf is used to write to the correct code buffer when needed.
    std::string* writeBuf = nullptr; 
    size_t index = 0;

    // loop through all lines of the shader source
    while (std::getline(shaderSource, line)) {
        
#ifdef _WIN32
        // Renderers line endings, strip the \r from the end of the line if it exists.
        // yeah no thanks Renderers. thanks. yeah thanks. thanks. thanks Renderers.
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

#endif
        // possibility 1: line is vertexIdentifer
        if (line == shader_VertexIdentifier) {
            
            // current line is the vertex identifier,
            // we are in the vertex source right now.
            // set the write buffer to write to vCode.
            writeBuf = &vCode; 
            // we have to write some glsl boilerplate
            // to the write buf.
            line = shader_vertexLayouts;
        }
        // possibility 2: line is fragmentIdentifier
        else if (line == shader_FragmentIdentifier) {

            // current line is the fragment identifier,
            // now we are in the fragment code.
            // same thing, set writeBuf to fCode.
            writeBuf = &fCode; 
            // add some more boilerplate we need
            line = shader_fragmentLayouts;
            line += shader_materialUniform;
        }
        // possibility 3: something else
        else {

            // so because of some in/out naming conflicions that glsl has,
            // as i explained in the shader header,
            // we need to change all references to the following:
            // v_position, v_colour, v_uv, v_normal
            // to a special new variable passed through from
            // the vertex source.
            // this has to happen because the v_ vars are passed into the
            // shader as vertex attribs and cannot be passed through to the
            // fragment shader without being renamed.
            
            // tldr; rename the v_ vars to keep the glsl compiler happy
            
            if (writeBuf == &fCode) {
                // find the first reference of a v_ var on this line 
                size_t subindex = line.find(shader_transpilerIdentifier);

                // while there are more v_ vars on this line:
                while (subindex != std::string::npos) {
                    // remove the v_ identifier, replace it with _transpilerID
                    line.replace(subindex, shader_transpilerIdentifier.length(), "_transpilerID_");
                    // try get the next reference of a v_ var on this line
                    subindex = line.find(shader_transpilerIdentifier, subindex + 15); 
                }
            }
        }

        // if the write buffer is actually valid, write the current line to it.
        if (writeBuf) {
            *writeBuf += line + "\n"; // remembering to add a new line identifier to it. this is just a string, after all
        }

        // increment the line index to the next line
        index++;
    }

    // insert the vertex out code at the end of the 
    // vertex main() function
    size_t lastVertBrace = vCode.find_last_of('}');
    if (lastVertBrace != std::string::npos) {
        // go back 1
        lastVertBrace -= 1;
        vCode.insert(lastVertBrace, shader_vertexOut);
    }

    // insert the transparency correction code at the 
    // end of the fragment main() function.
    size_t lastFragBrace = fCode.find_last_of('}');
    if (lastFragBrace != std::string::npos) {
        // go back 1
        lastFragBrace -= 1;
        fCode.insert(lastFragBrace, shader_fragmentFixTransparency);
    }

    //std::cout << "Shader code: " << vCode << std::endl << fCode << std::endl;

    // create the shader handles
    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);

    // store the code as char strings (easier)
    const char* vCode_cstr = vCode.c_str();
    const char* fCode_cstr = fCode.c_str();
    
    int vCodeLen = static_cast<int>(vCode.length());
    int fCodeLen = static_cast<int>(fCode.length());

    // assign the source code to the shader handles 
    glShaderSource(vert, 1, &vCode_cstr, &vCodeLen);
    glShaderSource(frag, 1, &fCode_cstr, &fCodeLen);

    // try compile the shaders
    glCompileShader(vert);
    glCompileShader(frag);

    // get any errors given by the glsl compiler
    std::string verror = _getShaderCompileErrors(vert);
    std::string ferror = _getShaderCompileErrors(frag);

    if (verror != "" || ferror != "") {
        // there have been errors, dump the shader glsl source
        log().logError("Shader dump:\nVertex:\n\n{}\n\nFragment:\n\n{}", vCode, fCode);
        log().logError("ShaderProgram with key '{}' has compilation errors!\nVertex Error(s):\n{}\nFragment Error(s):\n{}", m_key, verror, ferror);

        // cleanup shaders we don't need anymore
        glDeleteShader(vert);
        glDeleteShader(frag);
        
        // return appropriate error code
        return ERR_SHADER_COMPILATION_FAILED;
    }

    // compilation succeeded! create the program itself 
    m_programID = glCreateProgram();
    // attach our vertex and fragment shaders to this program
    // (think of it like the EBO and VBO of the VAO, but shaders)
    glAttachShader(m_programID, vert);
    glAttachShader(m_programID, frag);
    // link the program together.
    glLinkProgram(m_programID);

    char linkLog[512];
    int stat;
    glGetProgramiv(m_programID, GL_LINK_STATUS, &stat);
    if (!stat) {
        glGetProgramInfoLog(m_programID, 512, NULL, linkLog);
        log().logError("ShaderProgram with key '{}' link failed! Link log: {}", m_key, linkLog);
    }

    // cleanup, we are done with these. they are linked to the program now.
    glDeleteShader(vert);
    glDeleteShader(frag);

    // done!
    return SHADER_COMPILE_SUCCESS;
}

// check if this shader step has any compile errors. returns 0 if no errors.
std::string ShaderProgram::_getShaderCompileErrors(GLuint shader) {
    // store the success code
    int success;
    // get the compile status code
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        // get log length from opengl
        int logLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        
        if (logLength > 0) {
            // allocate a string with enough room to store the log
            std::string infoLog(logLength, '\0');
            
            // get the log
            glGetShaderInfoLog(shader, logLength, NULL, &infoLog[0]);
            return infoLog;
        }
        return "Unknown shader compilation error. Driver returned empty log.";
    }
    return "";
}

// returns a shared_ptr to an existing shader via a shader name.
// (if that shader exists, that is.)
std::shared_ptr<ShaderProgram> ShaderProgram::getShaderViaKey(std::string shaderKey) {
    // loop through cache, try find cacheID matching shader key
    auto it = stringToIDMap().find(shaderKey);
    if (it != stringToIDMap().end()) {
        
        // got a cache id, try get an instance 
        auto inst = ICachable::getCachedInstance(it->second);
        if (!inst) {
            // instance is null, shader must have been destroyed or is invalid.
            // delete this entry from the map
            stringToIDMap().erase(it);
            return nullptr;
        }
        return inst;
    }
    // no shader with that key in cache.
    return nullptr;
}

// get the location of a uniform with the help of the uniform cache.
// uniformLoc is just the name of a uniform within the shader.
// use this instead of glGetUniformLocation()
int ShaderProgram::getUniformLoc(const std::string &uniformLoc) {
    // look for uniform in uniform cache
    if (m_uniformCache.find(uniformLoc) != m_uniformCache.end()) {
        // found uniform location! return that
        return m_uniformCache[uniformLoc];
    }

    // no uniform location cached, generate one lol
    // get uniform location (expensive task)
    int loc = glGetUniformLocation(m_programID, uniformLoc.c_str());
    if (loc == -1) {
        // location doesn't exist
        //std::cerr << "ShaderProgram with key '" << m_key << "' Uniform '" << uniformLoc << "' doesn't exist!\n";
        return -1;
        // we could just add this incorrect location to the cache,
        // but then it might go unnoticed in development.
        // this way it tells us each frame that something is wrong
        // and forces us to fix it.
    } 
    // location exists! add it to the cache
    m_uniformCache.emplace(uniformLoc, loc);
    return loc;
}

// im not commenting allat
void ShaderProgram::setUniform(const std::string& uniformLoc, float uniform) {
    useProgram();
    glUniform1fv(getUniformLoc(uniformLoc), 1, &uniform);
}

void ShaderProgram::setUniform(const std::string& uniformLoc, int uniform) {
    useProgram();
    glUniform1iv(getUniformLoc(uniformLoc), 1, &uniform);
}

void ShaderProgram::setUniform(const std::string& uniformLoc, bool uniform) {
    setUniform(uniformLoc, static_cast<int>(uniform));
}

void ShaderProgram::setUniform(const std::string& uniformLoc, double uniform) {
    useProgram();
    glUniform1dv(getUniformLoc(uniformLoc), 1, &uniform);
}

void ShaderProgram::setUniform(const std::string& uniformLoc, unsigned int uniform) {
    useProgram();
    glUniform1uiv(getUniformLoc(uniformLoc), 1, &uniform);
}


void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_vec2<float>& uniform) {
    useProgram();
    glUniform2fv(getUniformLoc(uniformLoc), 1, &uniform.x);
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_vec3<float>& uniform) {
    useProgram();
    glUniform3fv(getUniformLoc(uniformLoc), 1, &uniform.x); 
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_vec4<float>& uniform) {
    useProgram();
    glUniform4fv(getUniformLoc(uniformLoc), 1, &uniform.x); 
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const fm::_mat4x4<float>& uniform) {
    useProgram();
    glUniformMatrix4fv(getUniformLoc(uniformLoc), 1, GL_FALSE, uniform.data()); 
}

// pointers are dangerous, so do a null check
void ShaderProgram::setUniform(const std::string& uniformLoc, const Texture2D* uniform) {
    if (!uniform) return;
    setUniform(uniformLoc, static_cast<int>(uniform->getUnitID()));
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const std::shared_ptr<Texture2D> uniform) {
    if (!uniform) return;
    setUniform(uniformLoc, uniform.get());
}
void ShaderProgram::setUniform(const std::string& uniformLoc, const Material* uniform) {
    if (!uniform) return;
    setUniform(uniformLoc + materialUniform_albedoFac,    uniform->m_data.m_albedoFac);
    setUniform(uniformLoc + materialUniform_emissionFac,  uniform->m_data.m_emissionFac);
    setUniform(uniformLoc + materialUniform_roughnessFac, uniform->m_data.m_roughnessFac);
    setUniform(uniformLoc + materialUniform_metallicFac,  uniform->m_data.m_metallicFac);
    setUniform(uniformLoc + materialUniform_ambientFac,   uniform->m_data.m_ambientFac);

    setUniform(uniformLoc + materialUniform_hasAlbedo,    uniform->m_data.m_hasAlbedoTx);
    setUniform(uniformLoc + materialUniform_hasNormal,    uniform->m_data.m_hasNormalTx);
    setUniform(uniformLoc + materialUniform_hasMetallic,  uniform->m_data.m_hasMetallicTx);
    setUniform(uniformLoc + materialUniform_hasEmission,  uniform->m_data.m_hasEmissionTx);
    setUniform(uniformLoc + materialUniform_hasTransp,    uniform->m_data.m_hasTransparency);

    setUniform(materialUniform_albedoTx,     uniform->getAlbedoTx());
    setUniform(materialUniform_normalTx,     uniform->getNormalTx());
    setUniform(materialUniform_metallicTx,   uniform->getMetallicTx());
    setUniform(materialUniform_emissionTx,   uniform->getEmissionTx());

}
void ShaderProgram::setUniform(const std::string& uniformLoc, const std::shared_ptr<Material> uniform) {
    if (!uniform) return;
    setUniform(uniformLoc, uniform.get());
}