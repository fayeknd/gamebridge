#pragma once
#include <engine.h>
#include <gl_context.hpp>
#include <fm/fmath.hpp>
#include <cachable.hpp>
#include <string>
#include <filesystem>
#include <unordered_map>
#include <memory>
#include <logger.hpp>
#include <texture.hpp>

// just an identifier for a shader with no key passed through.
#define UNNAMED_SHADER "0"

namespace Engine {

    class Material; // love a cheeky forward declaration

    // TODO : create a function to set default uniforms for a shader
    // to reduce verbosity / boilerplate

    // this whole section should really change

    // a constant for the layout position of the vertex position attrib.
    constexpr uint8_t shaderVertexAttrib_Position       = 0x00;

    // a constant for the layout position of the vertex colour attrib.
    constexpr uint8_t shaderVertexAttrib_Colour         = 0x01;

    // a constant for the layout position of the vertex uv attrib.
    constexpr uint8_t shaderVertexAttrib_UV             = 0x02;

    // a constant for the layout position of the vertex normal attrib.
    constexpr uint8_t shaderVertexAttrib_Normal         = 0x03;
    

    // a constant for the identifier for the "vertex" part of the shader.
    constexpr const char* shader_VertexIdentifier = "#VERTEX";

    // a constant for the identifier for the "fragment" part of the shader.
    constexpr const char* shader_FragmentIdentifier = "#FRAGMENT";

    // a constant for the model matrix
    constexpr const char* shader_ModelMatrix = "u_model_matrix";

    // a constant for the view/projection matrix
    constexpr const char* shader_ViewProjectionMatrix = "u_vp_matrix";

    // a constant for the default material
    constexpr const char* shader_defaultMaterial = "u_material";

    // the default lit shader.
    // if a shader doesn't have a shader key, it will just use the path as the key
    constexpr const char* shader_litShader = "lit_shader";
    constexpr const char* shader_unlitShader = "unlit_shader";
    constexpr const char* shader_colliderShader = "collider_shader";
    constexpr const char* shader_lineShader = "line_shader";

    // some pregenerated code.

    // the reason for the _transpilerID is significant.
    // in glsl, you cannot create an out variable with the same name
    // as an in variable for obvious reasons (no same name variables,
    // like any language).

    // ideally, it would be favourable to be able to call v_position 
    // in both the fragment and vertex shader to feel more seamless.
    // to combat this, the raw glsl out and in variables in both
    // the vertex and fragment shader are _transpilerID_var.
    // (transpilerIdentifier). 
    
    // the glsl transpiler will find any mentions of v_var (v_position,
    // v_colour, ...) and replace the v_ with _transpilerID_.
    // this is unnoticeable when using the custom glsl lang
    // but keeps the glsl compiler happy.

    // this code snippet just gets added to your vertex segment of the
    // shader file to remove some glsl verbosity that you don't need.

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

        "uniform mat4 u_model_matrix;\n"
        "uniform mat4 u_vp_matrix;\n"
    };


    // more stuff for reducing fragment shader verbosity
    constexpr const char* shader_fragmentLayouts = {
        "#version 460 core\n"
        "layout (location = 0) out vec4 fragCol;\n"
        "in vec3 _transpilerID_position;\n"
        "in vec4 _transpilerID_colour;\n"
        "in vec2 _transpilerID_uv;\n"
        "in vec3 _transpilerID_normal;\n\n"
    };

    // this gets added to the main function of the vertex shader
    // to assign the value of the out v_vars for the frag shader.
    constexpr const char* shader_vertexOut = {
        "\n    _transpilerID_position = gl_Position.xyz;\n"
        "    _transpilerID_colour = v_colour;\n"
        "    _transpilerID_uv = v_uv;\n"
        "    _transpilerID_normal = mat3(transpose(inverse(u_model_matrix))) * v_normal;\n"
    };

    constexpr const char* shader_fragmentFixTransparency = {
        "\n    fragCol.w = (u_material.has_transparency) ? fragCol.w : 1.0f;\n"
    };

    // the material struct definition in GLSL.
    // this will be within the vertex shader.
    constexpr const char* shader_materialUniform = {
        "struct pbrData {\n"
        "   vec4  albedo_fac;\n"
        "   vec4  emission_fac;\n"
        "   float roughness_fac;\n"
        "   float metallic_fac;\n"
        "   float ambient_fac;\n"
            
        "   bool  has_albedo_tx;\n"
        "   bool  has_normal_tx;\n"
        "   bool  has_metallic_tx;\n"
        "   bool  has_emission_tx;\n"
        "   bool  has_transparency;\n"
        "};\n\n"
        "uniform pbrData   u_material;\n\n"
        "uniform sampler2D u_albedo_tx;\n"
        "uniform sampler2D u_normal_tx;\n"
        "uniform sampler2D u_metallic_tx;\n"
        "uniform sampler2D u_emission_tx;\n"
    };

    // like i said earlier, the program finds any v_vars in the frag
    // shader with the identifier v_. this is just the const for that.
    const std::string shader_transpilerIdentifier = {"v_"};

    // enable_shared_from_this enables the use of the shared_from_this() function
    // which allows you to pass a "this" pointer as a shared_ptr.
    // useful for self-registry into the program cache. (s_programCache)
    class ShaderProgram : public ICachable<ShaderProgram> {
    private:

        static Logging::Logger& log() {
            static Logging::Logger instance("SHADER PROGRAM");
            return instance;
        }

        using ICachable::ICachable; 

        // the shader program ID for binding to opengl
        GLuint m_programID;

        // effectively the shader name. used for the unordered map.
        // this is not called the name because it is primarily used
        // as the key for the map.
        std::string m_key;

        // a map of string keys to cacheid keys.
        static std::unordered_map<std::string, CacheID>& stringToIDMap();

        // a cache of this shader's used uniforms. only a performance thing really.
        // glGetUniformLoc is an expensive function and storing the uniform locations
        // here prevents unnecessary performance loss.
        std::unordered_map<std::string, int> m_uniformCache;

        // compile the program via stringstream.
        // the reason we pass this as a stringstream is because 
        // stringstreams work with std::getline.
        // while we probably don't need to call this on it's own, 
        // if we want to and we have a string of shader code,
        // we can just write that to a stringstream via this snippet:

        // std::ifstream file(code);
        // std::stringstream buf;
        // buf << file.rdbuf(); // buf is the stringstream to pass
        int _compileProgram(std::stringstream& shaderSource);

        // check if this shader step has any compile errors. returns 0 if no errors.
        // returns 0 to allow for code such as:

        // auto vertexError = _getShaderCompileErrors(vshader);
        // if (vertexError) { //an error }
        // else { // no error }
        static std::string _getShaderCompileErrors(GLuint shader);

        // create program from a string stream. this string stream gets passed right through to _compileProgram().
        static std::shared_ptr<ShaderProgram> _createProgramFromStringStream(std::stringstream& ss, std::string shaderKey = UNNAMED_SHADER);

        // create some default shaders.
        static void _createDefaultShaders();

        friend class Renderer;

    public:

        ~ShaderProgram();

        // create program from a file on the disk.
        // can also be used to get a program from an archive
        // on the disk with the help of the resource manager.
        // the shader source is the path to the shader,
        // the shader key is the name to use to find the shader
        // in the shader cache.
        // for instance:
        // auto shader = ShaderProgram::createProgramFromPath("shaders/awesomeshader.sh", "MyCoolShader");
        // ShaderProgram::getShaderViaKey("MyCoolShader")->useProgram();
        // this will return null if the program failed to create,
        // and will return a shared_ptr to the program created
        // if successful.
        static std::shared_ptr<ShaderProgram> createProgramFromPath(std::filesystem::path shaderSource, std::string shaderKey = UNNAMED_SHADER);

        // create program from an unsigned char array of data.
        // to make it clear, an unsigned char array is the raw
        // file data loaded into memory. 
        // aside from that, functionally the same as the above function.
        static std::shared_ptr<ShaderProgram> createProgramFromData(unsigned char* data, size_t shaderSize, std::string shaderKey = UNNAMED_SHADER);

        // returns a shared_ptr to an existing shader via a shader name.
        // (if that shader exists, that is.)
        static std::shared_ptr<ShaderProgram> getShaderViaKey(std::string shaderKey);

        // returns this shader's key (or name)
        std::string getKey(); 

        // binds this shader program to the gl state if it is not already bound;
        // marks this program as the shader to use for drawing.
        void useProgram();

        // get the location of a uniform with the help of the uniform cache.
        // uniformLoc is just the name of a uniform within the shader.
        // use this instead of glGetUniformLocation()
        int getUniformLoc(const std::string& uniformLoc);

        // returns the program id.
        GLuint getProgramID() { return m_programID; }
        
        // sets a float uniform by name and value.
        void setUniform(const std::string& uniformLoc, float uniform);

        // sets an int uniform by name and value. 
        void setUniform(const std::string& uniformLoc, int uniform);

        // sets a boolean uniform by name and value. 
        void setUniform(const std::string& uniformLoc, bool uniform);

        // sets a double uniform by name and value. 
        void setUniform(const std::string& uniformLoc, double uniform);

        // sets an unsigned int uniform by name and value. 
        void setUniform(const std::string& uniformLoc, unsigned int uniform);


        // sets a vec2 uniform by name and value. 
        void setUniform(const std::string& uniformLoc, const fm::vec2& uniform);

        // sets a vec3 uniform by name and value. 
        void setUniform(const std::string& uniformLoc, const fm::vec3& uniform);

        // sets a vec4 uniform by name and value. 
        void setUniform(const std::string& uniformLoc, const fm::vec4& uniform);

        // sets a matrix uniform by name and value. 
        void setUniform(const std::string& uniformLoc, const fm::mat4& uniform);

        // sets a texture by name and value.
        void setUniform(const std::string& uniformLoc, const Texture2D* uniform);
        void setUniform(const std::string& uniformLoc, const std::shared_ptr<Texture2D> uniform);
      
        void setUniform(const std::string& uniformLoc, const Material* uniform);
        void setUniform(const std::string& uniformLoc, const std::shared_ptr<Material> uniform);
            
    };
}