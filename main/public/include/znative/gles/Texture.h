//
// Created on 2024/6/5.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#pragma once

#include <mutex>

#include <znative/base/RawData.h>
#include <znative/gles/GLUtil.h>

NAMESPACE_DEFAULT


struct TexParams {
    GLint magFilter = GL_LINEAR;
    GLint minFilter = GL_LINEAR;
    GLint wrapS = GL_CLAMP_TO_EDGE;
    GLint wrapT = GL_CLAMP_TO_EDGE;

    GLenum target = GL_TEXTURE_2D;
    GLint level = 0;
    GLint internalFormat = GL_RGBA;
    GLint border = 0;
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;
};

class Texture {
public:
    inline static GLuint INVALID_ID = GLUtil::INVALID_ID;
public:
    Texture() = default;
    Texture(GLuint id, GLint width, GLint height) : m_id(id), m_width(width), m_height(height) {}
    Texture(const Texture &other) : m_id(other.m_id), m_width(other.m_width), m_height(other.m_height) {}
    
    Texture& operator=(const Texture &other) {
        if (this != &other) {
            m_id = other.m_id;
            m_width = other.m_width;
            m_height = other.m_height;
        }
        return *this;
    }

    inline GLuint id() const { return m_id; }
    inline GLint width() const { return m_width; }
    inline GLint height() const { return m_height; }

    inline bool valid() const { return m_id != INVALID_ID && m_width > 0 && m_height > 0; }

protected:
    GLuint m_id = INVALID_ID;
    GLint m_width = 0;
    GLint m_height = 0;
};

class Texture2D : public Texture {
public:
    static std::shared_ptr<Texture2D> create(GLuint width = 0, GLuint height = 0, const TexParams &params = {}) {
        return std::shared_ptr<Texture2D>(new Texture2D(width, height, params), [](Texture2D *tex) {
            tex->release();
            delete tex;
        });
    }

    static std::shared_ptr<Texture2D> create(GLuint width, GLuint height, const void *pixels, const TexParams &params = {}, int unpackRowLength = -1, int unpackAlignment = -1) {
        auto ptr = std::shared_ptr<Texture2D>(new Texture2D(width, height, params), [](Texture2D *tex) {
            tex->release();
            delete tex;
        });
        ptr->update(pixels, unpackRowLength, unpackAlignment);
        return ptr;
    }

    /**
     * 封装已存在的纹理对象
     * @param id 纹理ID
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param params 纹理参数
     * @param owner 是否拥有纹理ID的所有权, 如果为 true, 则在析构时释放纹理ID
     * @return 封装后的纹理对象
     */
    static std::shared_ptr<Texture2D> wrap(GLuint id, GLint width, GLint height, const TexParams &params = {}, bool owner = false) {
        return std::shared_ptr<Texture2D>(new Texture2D(id, width, height, params), [owner](Texture2D *tex) {
            if (owner) {
                tex->release();
            }
            delete tex;
        });
    }

    static int defaultFormatAlignment(GLenum format) {
        switch (format) {
#ifdef GL_RED
        case GL_RED:
#endif
#ifdef GL_GREEN
        case GL_GREEN:
#endif
#ifdef GL_BLUE
        case GL_BLUE:
#endif
#ifdef GL_ALPHA
        case GL_ALPHA:
#endif
        case GL_LUMINANCE:
            return 1;
        case GL_LUMINANCE_ALPHA:
            return 2;
#ifdef GL_BGR
        case GL_BGR:
#endif
        case GL_RGB:
            // Alignment 必须是 1,2,4, 所以即使是 GL_RGB 也需要设置为 4
#ifdef GL_BGRA
        case GL_BGRA:
#endif
        case GL_RGBA:
            return 4;
        default:
            return 4;
        }
    }

private:
    Texture2D(GLint width, GLint height, const TexParams &params = {})
        : Texture(INVALID_ID, width, height), m_params(params) {}
    Texture2D(GLuint id, GLint width, GLint height, const TexParams &params)
        : Texture(id, width, height), m_params(params) {}

public:
    // 注意必须手动调用 release() 释放纹理
    ~Texture2D() = default;

public:
    /**
     * 更新2D纹理像素数据
     * @param pixels 纹理像素数据
     * @param unpackRowLength 解包行长度字节数, 默认-1，表示为0
     * @param unpackAlignment 解包对齐字节数, 默认-1，根据纹理格式自动计算对齐字节数
     */
    void update(const void *pixels, int unpackRowLength = -1, int unpackAlignment = -1) {
        if (valid()) {
            updateTexture2D(m_id, m_width, m_height, m_params, pixels, unpackRowLength, unpackAlignment);
        } else {
            m_id = genTexture2D(m_width, m_height, m_params, pixels, unpackRowLength, unpackAlignment);
            _INFO("Texture2D created: %d, %d, %d", m_id, m_width, m_height);
        }
    }

    /**
     * 检查并更新2D纹理像素数据,
     * 如果纹理宽度或高度与指定宽度或高度不同, 则释放纹理并重新创建纹理,
     * @param pixels 纹理像素数据
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param unpackRowLength 解包行长度字节数, 默认-1，表示为0
     * @param unpackAlignment 解包对齐字节数, 默认-1，根据纹理格式自动计算对齐字节数
     */
    void checkUpdate(const void *pixels, int width, int height, int unpackRowLength = -1, int unpackAlignment = -1) {
        if (this->m_width != width || this->m_height != height) {
            release();
            m_width = width;
            m_height = height;
        }
        if (width > 0 && height > 0) {
            this->update(pixels, unpackRowLength, unpackAlignment);
        }
    }

    bool isSameConfig(int w, int h, const TexParams& p) const {
        return m_width == w && m_height == h && memcmp(&m_params, &p, sizeof(TexParams)) == 0;
    }

    const TexParams& params() const {
        return m_params;
    }

    void release() {
        if (m_id != INVALID_ID) {
            glDeleteTextures(1, &m_id);
            m_id = INVALID_ID;
        }
    }

public:
    /**
     * 更新2D纹理像素数据
     * @param id 纹理ID
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param params 纹理参数
     * @param pixels 纹理像素数据
     * @param unpackRowLength 解包行长度
     * @param unpackAlignment 解包对齐字节数
     */
    static void updateTexture2D(GLuint id, GLint width, GLint height, const TexParams &params, const void *pixels, int unpackRowLength = -1, int unpackAlignment = -1) {
        glBindTexture(GL_TEXTURE_2D, id);
        int originalAlignment = 0, originalRowLength = 0;
        setFormatUnpackParameter(params.format, unpackRowLength, unpackAlignment, originalRowLength, originalAlignment);
        if (unpackAlignment > 0 && (width * height)%unpackAlignment != 0) {
            _ERROR("Error update texture, size not divisible by alignment, id: %u, width: %d, height: %d, alignment: %d", id, width, height, unpackAlignment);
        }
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, params.format, params.type, pixels);
        restoreUnpackParameter(originalRowLength, originalAlignment);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    /**
     * 生成2D纹理
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param params 纹理参数
     * @param pixels 纹理像素数据
     * @param unpackRowLength 解包行长度
     * @param unpackAlignment 解包对齐 -1 时会根据格式默认对齐字节数
     * @return 纹理ID
     */
    static GLuint genTexture2D(GLint width, GLint height, const TexParams &params, const void *pixels, int unpackRowLength = -1, int unpackAlignment = -1) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, params.magFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, params.minFilter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, params.wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, params.wrapT);
        int originalAlignment = 0, originalRowLength = 0;
        setFormatUnpackParameter(params.format, unpackRowLength, unpackAlignment, originalRowLength, originalAlignment);
        if (unpackAlignment > 0 && (width * height)%unpackAlignment != 0) {
            _ERROR("Error generate texture, size not divisible by alignment, id: %u, width: %d, height: %d, alignment: %d", texture, width, height, unpackAlignment);
        }
        glTexImage2D(GL_TEXTURE_2D, params.level, params.internalFormat, width, height, params.border, params.format,
                     params.type, pixels);
        restoreUnpackParameter(originalRowLength, originalAlignment);
        glBindTexture(GL_TEXTURE_2D, 0);
        return texture;
    }

    /**
     * 设置纹理解包参数
     * @param format 纹理格式
     * @param rowLength 原始行长度，< 1 表示设置为 0
     * @param alignment 解包对齐字节数
     * @param originalRowLength 原始行长度， 如果不需要 restore 返回 -1
     * @param originalAlignment 返回之前设置的对齐字节数, 如果不需要 restore 返回 -1
     */
    static void setFormatUnpackParameter(
        int format, int& rowLength, int& alignment,
        int &originalRowLength, int &originalAlignment) {
        originalAlignment = -1;
        glGetIntegerv(GL_UNPACK_ALIGNMENT, &originalAlignment);
        if (alignment < 1) {
            alignment = defaultFormatAlignment(format);
        }
        // 核心：设置 1 字节对齐（解决宽度不被4整除的问题）
        if (originalAlignment != alignment) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        } else {
            // 不需要 restore
            originalAlignment = -1;
        }

        originalRowLength = -1;
        glGetIntegerv(GL_UNPACK_ROW_LENGTH, &originalRowLength);
        if (rowLength > 0 && rowLength != originalRowLength) {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
        } else if (originalRowLength > 0) {
            // 避免影响我们的纹理读取
            glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
        } else {
            // 不需要 restore
            originalRowLength = -1;
        }
    }

    static void restoreUnpackParameter(int rowLength, int alignment) {
        // _INFO("restoreUnpackParameter: rowLength: %d, alignment: %d", rowLength, alignment);
        if (rowLength >= 0) {
            glPixelStorei(GL_UNPACK_ROW_LENGTH, rowLength);
        }
        if (alignment > 0) {
            glPixelStorei(GL_UNPACK_ALIGNMENT, alignment);
        }
    }

private:
    TexParams m_params;
};

class ImageTexture {
public:
    ~ImageTexture() {
        if (valid()) {
            _WARN("ImageTexture not released before destroyed!");
        }
    }

    /**
     * 异步更新纹理像素数据，先将数据拷贝到内部缓存，后续在渲染时更新纹理
     * @param data 纹理像素数据
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param params 纹理参数
     * @param bytesPerRow 每行字节数，默认-1, 表示没有任何 padding
     */
    void put(const uint8_t * data, int width, int height,
             const TexParams& params = {}, int bytesPerRow = -1) {
        int channels = Texture2D::defaultFormatAlignment(params.format);

        std::lock_guard<std::mutex> lock(m_put_mutex);
        int dataSize = width * height * channels;
        if (bytesPerRow > 0) {
            dataSize = bytesPerRow * height;
        }
        if (m_next_img.size() != dataSize) {
            m_next_img = RawData(dataSize);
        }
        memcpy(m_next_img.data(), data, dataSize);
        m_next_width = width;
        m_next_height = height;
        m_next_params = params;
        m_next_bytesPerRow = bytesPerRow;
        m_tex_need_update = true;
    }

    bool valid() const {
        return m_tex && m_tex->valid();
    }

    /**
     * 直接生成纹理，更新纹理像素数据
     * @param data 纹理像素数据
     * @param width 纹理宽度
     * @param height 纹理高度
     * @param params 纹理参数
     * @param bytesPerRow 每行字节数，默认-1, 表示没有任何 padding
     * @return 纹理指针
     */
    std::shared_ptr<Texture2D> update(const uint8_t * data, int width, int height,
                                      const TexParams& params = {}, int bytesPerRow = -1) {
        std::lock_guard<std::mutex> lock(m_put_mutex);

        if (m_tex == nullptr || !m_tex->isSameConfig(width, height, params)) {
            m_tex = Texture2D::create(width, height, params);
        }

        m_tex->update(data, bytesPerRow, -1);
        m_tex_need_update = false;
        return m_tex;
    }

    Texture2D& textureNonnull() {
        std::shared_ptr<Texture2D> tex = texture();
        _FATAL_IF(!tex, "texture is nullptr!!")
        return *tex;
    }

     std::shared_ptr<Texture2D> texture() {
        if (!m_tex_need_update) {
            return m_tex;
        }
        if (m_next_width == 0 || m_next_height == 0) {
            _WARN("image not set! get texture failed!");
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(m_put_mutex);
        if (m_tex == nullptr || !m_tex->isSameConfig(m_next_width, m_next_height, m_next_params)) {
            m_tex = Texture2D::create(m_next_width, m_next_height, m_next_params);
            m_tex_need_update = true;
        }
        m_tex->update(m_next_img.data(), m_next_bytesPerRow, -1);
        m_tex_need_update = false;
        return m_tex;
    }

    void release() {
        std::lock_guard<std::mutex> lock(m_put_mutex);
        if (m_tex) {
            m_tex->release();
        }
        m_tex = nullptr;
        m_next_img = {};
    }

private:
    RawData m_next_img;
    int m_next_width = 0;
    int m_next_height = 0;
    TexParams m_next_params;
    int m_next_bytesPerRow = -1;

    std::shared_ptr<Texture2D> m_tex = nullptr;
    bool m_tex_need_update = false;

    std::mutex m_put_mutex;
};

NAMESPACE_END