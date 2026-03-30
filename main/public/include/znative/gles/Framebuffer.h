//
// Created on 2024/6/6.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#pragma once

#include <znative/gles/GLUtil.h>
#include <znative/gles/Texture.h>
#include <optional>

NAMESPACE_DEFAULT

class Framebuffer {
public:
    Framebuffer() = default;
    // wrap existing framebuffer
    explicit Framebuffer(GLuint fbId) : m_fb_id(fbId) {}

    void create(GLint width, GLint height, const TexParams &params = {}) {
        createColorTexture(width, height, params);
    }

    inline bool available() const { return m_ref_count == 0 && valid(); }

    inline void ref() { m_ref_count += 1; }

    inline void unref() {
        _WARN_RETURN_IF(m_ref_count == 0, void(), "Framebuffer::unref() ref count == 0, can't unref")
        m_ref_count -= 1;
    }

    inline bool valid() const { return m_color_texture != nullptr && m_fb_id.has_value(); }

    inline GLuint id() const { return m_fb_id.has_value() ? m_fb_id.value() : 0; }

    inline std::shared_ptr<Texture2D> texture() const { return m_color_texture; }

    inline const Texture2D& textureNonnull() const {
        _FATAL_IF(!m_color_texture, "texture of framebuffer is nullptr");
        return *m_color_texture;
    }

    inline GLuint texID() const { return m_color_texture == nullptr ? Texture::INVALID_ID : m_color_texture->id(); }

    inline GLint texWidth() const { return m_color_texture == nullptr ? 0 : m_color_texture->width(); }

    inline GLint texHeight() const { return m_color_texture == nullptr ? 0 : m_color_texture->height(); }
    
    bool copyTexTo(GLuint dstId) const {
        _ERROR_RETURN_IF(!valid(), false, "source frame buffer bind failed!")
        
        glBindFramebuffer(GL_FRAMEBUFFER, id());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, dstId);
        glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, texWidth(), texHeight());
        
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }
    
    bool copyTo(Framebuffer& dst) const {
        _FATAL_IF(!valid(), "source frame buffer invalid!");
        _FATAL_IF(!dst.valid(), "destination frame buffer invalid!");
        _FATAL_IF(texWidth() != dst.texWidth() || texHeight() != dst.texHeight(),
                  "source and destination frame buffer size not equal!");
        return this->copyTexTo(dst.texID());
    }

    void bind() {
        _FATAL_IF(!valid(), "Framebuffer::bind() invalid framebuffer");
        glBindFramebuffer(GL_FRAMEBUFFER, m_fb_id.value());
        ref();
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        unref();
    }

    std::shared_ptr<uint8_t> readPixels() {
        bind();
        GLint width = m_color_texture->width();
        GLint height = m_color_texture->height();
        std::shared_ptr<uint8_t> pixels = std::shared_ptr<uint8_t>(
            new uint8_t[width * height * 4], [](uint8_t *p) { delete[] p; });
        glReadPixels(0, 0, width, height, m_color_texture->params().format, GL_UNSIGNED_BYTE, pixels.get());
        unbind();

        return pixels;
    }

    void readPixelsTo(void* pixels) {
        bind();
        GLint width = m_color_texture->width();
        GLint height = m_color_texture->height();
        glReadPixels(0, 0, width, height, m_color_texture->params().format, GL_UNSIGNED_BYTE, pixels);
        unbind();
    }

    void release() {
        detachColorTexture();
        if (m_fb_id.has_value()) {
            GLuint value = m_fb_id.value();
            _INFO("Framebuffer(%d)::release()", value);
            glDeleteFramebuffers(1, &value);
            m_fb_id.reset();
            _WARN_IF(m_ref_count > 0, "Framebuffer(%d)::release() ref count: %d > 0", value, m_ref_count);
        }
    }

public:
    GLuint createFbId() {
        if (!m_fb_id.has_value()) {
            GLuint fbId;
            glGenFramebuffers(1, &fbId);
            m_fb_id = fbId;
        }
        return m_fb_id.value();
    }

    void createColorTexture(int width, int height, const TexParams &params = {}) {
        if (m_color_texture == nullptr || m_color_texture->width() != width || m_color_texture->height() != height) {
            _INFO("Framebuffer::createColorTexture(%d, %d)", width, height);
            attachColorTexture(Texture2D::create(width, height, nullptr, params));
        }
    }

    void attachColorTexture(const std::shared_ptr<Texture2D>& texture) {
        _FATAL_IF(texture == nullptr || !texture->valid(), "Framebuffer::attachColorTexture failed, texture invalid!")
        GLuint fbId = createFbId();

        if (m_color_texture != nullptr) {
            detachColorTexture();
        }

        glBindFramebuffer(GL_FRAMEBUFFER, fbId);
        glBindTexture(GL_TEXTURE_2D, texture->id());
        // set texture as colour attachment
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture->id(), 0);

        // unbind
        glBindTexture(GL_TEXTURE_2D, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        this->m_color_texture = texture;
        _INFO("Framebuffer(%d)::attachColorTexture(%d)", id(), m_color_texture->id());
    }

    void detachColorTexture() {
        if (m_color_texture == nullptr) {
            return;
        }

        _INFO("Framebuffer(%d)::detachColorTexture(%d)", id(), m_color_texture->id());
        if (m_fb_id.has_value()) {
            glBindFramebuffer(GL_FRAMEBUFFER, m_fb_id.value());
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        m_color_texture = nullptr;
    }

private:
    std::optional<GLuint> m_fb_id;
    std::shared_ptr<Texture2D> m_color_texture = nullptr;

    int m_ref_count = 0;
};

NAMESPACE_END