#pragma once
#include <cachable.hpp>
#include <gl_context.hpp>
#include <fm/fmath.hpp>

namespace Engine {

    constexpr GLuint texture_defaultFiltering = GL_NEAREST;
    constexpr GLuint texture_defaultFormat    = GL_RGBA;

    class Texture : public ICachable<Texture> {
    protected:

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

        // texture size in pixels
        fm::uvec2 m_txSize;

        // just cheeky little path storage
        std::string m_path;

    public:

        // in previous engines, this would have cached the path,
        // and if it was called again with the same path,
        // would have returned a pointer to the already created
        // texture. this time, it's just going to create a new 
        // instance. trust the programmer. maybe we want differently
        // filtered textures.
        std::shared_ptr<Texture> createTextureFromPath(std::filesystem::path m_path, GLuint format = texture_defaultFormat, GLuint filtering = texture_defaultFiltering, GLint internalFormat = texture_defaultFormat);

        // get the texture ID
        GLuint getTextureID() { return m_txID; }

        // get the texture format
        GLuint getFormat() { return m_format; }

        // get the internal texture format
        GLuint getInternalFormat() { return m_internalFormat; }

        // how many channels does this texture use?
        GLuint getChannelCount() { return m_channelCount; }

        // what filtering method does this texture use?
        GLuint getFiltering() { return m_filtering; }

    };
}