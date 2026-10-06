#pragma once
#include "../engine.h"
#include "../gl/gl_context.hpp"

#include <string>
#include <filesystem>
#include <unordered_map>

#define UNNAMED_SHADER "0"

namespace Engine {

    constexpr uint8_t shaderVertexAttrib_Position       = 0x00;
    constexpr uint8_t shaderVertexAttrib_Colour         = 0x01;
    constexpr uint8_t shaderVertexAttrib_UV             = 0x02;
    constexpr uint8_t shaderVertexAttrib_Normal         = 0x03;
    
    const std::string shader_VertexIdentifier = "#VERTEX";
    const std::string shader_FragmentIdentifier = "#FRAGMENT";
    const std::string shader_GLVersion = "#version 460 core\n";

    // lowkey id be lying if i said i fully grasped what weak_ptrs were and why they work
    // this design is based off of cocos2d-x. the cleanup() function will find any shaders 
    // with no references and destroy them. a garbage collector. In some instances, 
    // you might not want shaders to be completely wiped out, so it's good top have the
    // option. Alternatively, if you were certain you wanted an object to be destroyed when
    // it's references drop to zero, you can just use a shared_ptr.

    class ShaderProgram : public std::enable_shared_from_this<ShaderProgram> {
    private:

        GLuint m_programID;
        bool m_registered = false;
        std::string m_key;

        static std::unordered_map<std::string, std::weak_ptr<ShaderProgram>> s_programCache; 
        int _compileProgram(std::filesystem::path shaderSource);
        static const char* _getShaderCompileErrors(GLuint shader);
        
        void _registerProgram();
        void _unregisterProgram();

    public:
        
        static void cleanup();

        static std::shared_ptr<ShaderProgram> createProgram(std::filesystem::path shaderSource, std::string shaderKey = UNNAMED_SHADER);
        static std::shared_ptr<ShaderProgram> getShaderViaKey(std::string shaderKey);

        void useProgram();
        
        ~ShaderProgram() {
            _unregisterProgram(); 
        }

    };

}