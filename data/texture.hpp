#pragma once
#include "idpool.hpp"
#include <cachable.hpp>
#include <gl_context.hpp>
#include <fm/fmath.hpp>
#include <resources.hpp>
#include <unordered_map>

namespace Engine {

    constexpr GLuint texture_defaultFiltering = GL_NEAREST;
    constexpr GLuint texture_defaultFormat    = GL_RGBA;
    constexpr GLuint texture_defaultWrapping  = GL_REPEAT;

    class Texture2D : public ICachable<Texture2D> {
    protected:

        static Logging::Logger& log() {
            static Logging::Logger instance("TEXTURE");
            return instance;
        }

        // a map of this texture id to the cache id
        static std::unordered_map<GLuint, CacheID>& txIDToIDMap();

        // the id pool for the bound texture units.
        // when a texture is queued, it has to get
        // the next free texture unit ID. 
        static inline IDPool<GLuint> s_unitPool;

        GLuint m_unitID;

        // has this texture been generated?
        bool m_txGenerated = false;

        // the texture ID
        GLuint m_txID;

        // format, like GL_RGB
        GLuint m_format = texture_defaultFormat;

        // internal format that determines how OpenGL handles this.
        // i.e GL_RGBA32. we hopefully won't have to change this ever,
        // but there might be an edge case that this would fix.
        GLuint m_internalFormat = texture_defaultFormat;

        // how many channels does this image have?
        GLuint m_channelCount;

        // texture filtering mode
        GLuint m_filtering = texture_defaultFiltering;

        // texture wrap mode
        GLuint m_wrapping = texture_defaultWrapping;

        // type of texture
        GLuint m_txType = GL_TEXTURE_2D;

        // texture size in pixels
        fm::uvec2 m_txSize;

        // determines whether this texture is mipmapped or not
        bool m_useMipmapping = true;

        // is this texture in the texture queue?
        bool m_inQueue = false;

        // just cheeky little path storage
        std::string m_path;

        static bool _processRawBytes(bytes& outBytes, std::shared_ptr<Texture2D> tx);
        
        // does not follow the same structure as shader.hpp does. 
        // this is because this function does not create a texture,
        // it just modifies an existing one.
        // this is because stbi_load_from_memory sets some texture
        // values as it does it, so we need to account for that.
        void _initTextureWithSTBIData(bytes& data, bool mipmap = true, GLuint format = texture_defaultFormat, GLuint filtering = texture_defaultFiltering, GLuint internalFormat = texture_defaultFormat, GLuint wrapping = texture_defaultWrapping, GLuint type = GL_TEXTURE_2D);

        // generate a texture from all the values on this texture (+data).
        void _genTexture(bytes& data); 

        // add / remove this texture from the global texture queue
        void _queue();
        void _unqueue();

    public:

        ~Texture2D();

        // in previous engines, this would have cached the path,
        // and if it was called again with the same path,
        // would have returned a pointer to the already created
        // texture. this time, it's just going to create a new 
        // instance. trust the programmer. maybe we want differently
        // filtered textures.
        static std::shared_ptr<Texture2D> createTextureFromPath(std::filesystem::path path, bool flipY = true, bool mipmap = true, GLuint format = texture_defaultFormat, GLuint filtering = texture_defaultFiltering, GLuint internalFormat = texture_defaultFormat, GLuint wrapping = texture_defaultWrapping, GLuint type = GL_TEXTURE_2D);

        // idk why you would want to check this but sure have a getter
        const bool isGenerated() const { return m_txGenerated; }

        // TODO : setters

        // get the texture ID
        const GLuint getTextureID() const { return m_txID; }

        // get the texture format
        const GLuint getFormat() const { return m_format; }

        // get the internal texture format
        const GLuint getInternalFormat() const { return m_internalFormat; }

        // how many channels does this texture use?
        const GLuint getChannelCount() const { return m_channelCount; }

        // what filtering method does this texture use?
        const GLuint getFiltering() const { return m_filtering; }

        // what wrapping method does this texture use?
        const GLuint getWrapping() const { return m_wrapping; }

        // get the type of texture this is. for instance: GL_TEXTURE, GL_RENDER_TEXTURE, etc
        const GLuint getTextureType() const { return m_txType; }

        std::shared_ptr<Texture2D> getTextureViaID(GLuint tid);

        // is this texture using mipmapping?
        const bool isMipmapped() const { return m_useMipmapping; }

        // bind this texture.
        void bindTexture();

        // is this texture in the texture queue?
        const bool isInQueue() const { return m_inQueue; }

        const GLuint getUnitID() const { return m_unitID; }

    };
}