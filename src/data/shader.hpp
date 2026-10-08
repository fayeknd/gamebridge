#pragma once
#include "../engine.h"
#include "../gl/gl_context.hpp"
#include "../../include/fm/fmath.hpp"

#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>

#define UNNAMED_SHADER "0"

namespace Engine {

    constexpr uint8_t shaderVertexAttrib_Position       = 0x00;
    constexpr uint8_t shaderVertexAttrib_Colour         = 0x01;
    constexpr uint8_t shaderVertexAttrib_UV             = 0x02;
    constexpr uint8_t shaderVertexAttrib_Normal         = 0x03;
    
    constexpr const char* shader_VertexIdentifier = "#VERTEX";
    constexpr const char* shader_FragmentIdentifier = "#FRAGMENT";

    constexpr const char* shader_vertexLayouts = {
        "#version 460 core\n"
        "layout (location = 0) in vec3 v_position;\n"
        "layout (location = 1) in vec4 v_colour;\n"
        "layout (location = 2) in vec2 v_uv;\n"
        "layout (location = 3) in vec3 v_normal;\n"
        "out vec3 _transpilerID_position;\n"
        "out vec4 _transpilerID_colour;\n"
        "out vec2 _transpilerID_uv;\n"
        "out vec3 _transpilerID_normal;\n"
    };
    constexpr const char* shader_fragmentLayouts = {
        "#version 460 core\n"
        "layout (location = 0) out vec4 fragCol;\n"
        "in vec3 _transpilerID_position;\n"
        "in vec4 _transpilerID_colour;\n"
        "in vec2 _transpilerID_uv;\n"
        "in vec3 _transpilerID_normal;\n"
    };
    constexpr const char* shader_vertexOut = {
        "\n    _transpilerID_position = v_position;\n"
        "    _transpilerID_colour = v_colour;\n"
        "    _transpilerID_uv = v_uv;\n"
        "    _transpilerID_normal = v_normal;\n"
    };
    const std::string shader_transpilerIdentifier = {"v_"};

    // FUCK WEAK_PTR
    
    class ShaderProgram : public std::enable_shared_from_this<ShaderProgram> {
    private:

        GLuint m_programID;
        bool m_registered = false;
        std::string m_key;

        static inline std::unordered_map<std::string, std::shared_ptr<ShaderProgram>> s_programCache; 
        std::unordered_map<std::string, int> m_uniformCache;

        int _compileProgram(std::stringstream& shaderSource);
        static const char* _getShaderCompileErrors(GLuint shader);
        
        void _registerProgram();
        void _unregisterProgram();
        static std::shared_ptr<ShaderProgram> _createProgramFromStringStream(std::stringstream& ss, std::string shaderKey = UNNAMED_SHADER);

    public:
        
        static void cleanup();

        static std::shared_ptr<ShaderProgram> createProgramFromPath(std::filesystem::path shaderSource, std::string shaderKey = UNNAMED_SHADER);
        static std::shared_ptr<ShaderProgram> createProgramFromData(unsigned char* data, std::string shaderKey = UNNAMED_SHADER);
        static std::shared_ptr<ShaderProgram> getShaderViaKey(std::string shaderKey);

        std::string getKey(); 

        void useProgram();
        
        ~ShaderProgram() {
            _unregisterProgram(); 
        }

        int getUniformLoc(const std::string& uniformLoc);
        GLuint getProgramID() { return m_programID; }
        
        void setUniform(const std::string& uniformLoc, float uniform);
        void setUniform(const std::string& uniformLoc, int uniform);
        void setUniform(const std::string& uniformLoc, bool uniform);
        void setUniform(const std::string& uniformLoc, double uniform);
        void setUniform(const std::string& uniformLoc, unsigned int uniform);
        
        void setUniform(const std::string& uniformLoc, const fm::vec2& uniform);
        void setUniform(const std::string& uniformLoc, const fm::vec3& uniform);
        void setUniform(const std::string& uniformLoc, const fm::vec4& uniform);
        void setUniform(const std::string& uniformLoc, const fm::mat4& uniform);

        //void setUniform(std::string uniformLoc, Texture2D uniform); later
         
    };

}