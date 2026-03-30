//
// Created on 2024/6/5.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#pragma once

#include <znative/ZBase.h>
#include <znative/ZLog.h>

#include <cstring>

#ifdef ZNATIVE_GL_HEADER
#include ZNATIVE_GL_HEADER
#else
// 如果使用了 angle，使用 GLES3 的头文件
// 整个 gles 模块都是头文件，所以可以自定义使用哪个头文件
#if defined(__ANDROID__) || defined(__HARMONYOS__)
#include <GLES3/gl3.h>
#elif (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1)
#include <OpenGLES/ES3/gl.h>
#include <OpenGLES/ES3/glext.h>
#elif defined(__MACOS__) || defined(__APPLE__)
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/gl.h>
#include <OpenGL/gl3.h>
#else
#ifdef _WIN32
#include <GL/glew.h>
#else
#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include <OpenGL/gl.h>
#include <OpenGL/gl3.h>
#endif
#endif
#endif

NAMESPACE_DEFAULT

#define CHECK_GL_ERROR { GLenum en = glGetError(); if (en != GL_NO_ERROR) { _ERROR("find gl error: %d", en); }}

#ifndef GL_CORE_VERSION
#define GL_CORE_VERSION 330
#endif

#if defined(GL_GLES_PROTOTYPES) || defined(GL_ES_VERSION_3_0) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1) || defined(__IOS__) || defined(__HARMONYOS__) || defined(__ANDROID__)
#define CORRECT_VERTEX_SHADER(shader) shader
#define CORRECT_FRAGMENT_SHADER(shader) shader
#else
#define CORRECT_VERTEX_SHADER(shader) GLUtil::simpleConvertGLESShaderToGL(shader, true)
#define CORRECT_FRAGMENT_SHADER(shader) GLUtil::simpleConvertGLESShaderToGL(shader, false)
#endif

// 内部格式使用 GL_R8 代替 GL_LUMINANCE
#define GL_LUM_INTERNAL_FORMAT GL_R8
// GL_LUMINANCE 已经被弃用，使用 GL_RED 代替， 如果 opengl 版本 >= 3.3 必须使用 GL_RED, 否则报 1280 错误
#ifdef GL_ES_VERSION_2_0
#define GL_LUM_FORMAT GL_LUMINANCE
#else
#define GL_LUM_FORMAT GL_RED
#endif

// 内部格式使用 GL_RG8 代替 GL_LUMINANCE_ALPHA
#define GL_LUM_ALPHA_INTERNAL_FORMAT GL_RG8
// GL_LUMINANCE_ALPHA 已经被弃用，使用 GL_RG 代替
#if defined(GL_ES_VERSION_3_0) || defined(GL_ES_VERSION_2_0)
    #define GL_LUM_ALPHA_FORMAT GL_LUMINANCE_ALPHA
#else
    #define GL_LUM_ALPHA_FORMAT GL_RG
#endif

// trick 逻辑: 强制清空glGetError，避免glGetError拿到之前的错误阻碍资源创建
#define Z_CHECK_GL_ERROR do { GLenum error; while((error = glGetError()) != GL_NO_ERROR) { _ERROR("GL error: %s", znative::GLUtil::errorString(error)); }} while (0)

class GLUtil {
public:
    // 对于 纹理 id/程序 id 等，无效值为 0, 对于 framebuffer id 来说 0 是有效的
    static inline GLuint INVALID_ID = 0;
public:
    /**
     * 简单的转换，将 attribute 替换为 in，将 varying 替换为 out, 将 gl_FragColor 替换为 fragColor
     */
    static std::string simpleConvertGLESShaderToGL(std::string glesShader,
                                                   bool vertexOrFragment,
                                                   const std::string& glCoreVersion = "330") {
        std::size_t index = 0;
        while ((index = glesShader.find("attribute", index)) != std::string::npos) {
            glesShader.replace(index, 9, "in");
            index += 2;
        }
        index = 0;
        while ((index = glesShader.find("varying", index)) != std::string::npos) {
            if (vertexOrFragment) {
                glesShader.replace(index, 7, "out");
            } else {
                glesShader.replace(index, 7, "in");
            }
            index += 2;
        }

        // 将 precision highp float; 注释掉


        // texture2D -> texture
        index = 0;
        while ((index = glesShader.find("texture2D", index)) != std::string::npos) {
            glesShader.replace(index, 9, "texture");
            index += 7;
        }

        while ((index = glesShader.find("gl_FragColor")) != std::string::npos) {
            glesShader.replace(index, 12, "_out_fragColor");
        }

        if ((index = glesShader.find("_out_fragColor")) != std::string::npos) {
            glesShader = "out vec4 _out_fragColor;\n" + glesShader;
        }

        glesShader = "#version " + std::to_string(GL_CORE_VERSION) + " core\n" + glesShader;
        return glesShader;
    }

    static int glMajorVersion() {
        int majorVersion;
        glGetIntegerv(GL_MAJOR_VERSION, &majorVersion);
        return majorVersion;
    }

    static int glMinorVersion() {
        int minorVersion;
        glGetIntegerv(GL_MINOR_VERSION, &minorVersion);
        return minorVersion;
    }

    static std::string glVersion() {
        return (const char *)glGetString(GL_VERSION);
    }

    static GLuint loadShader(const std::string& str, int type) {
        GLuint shader = glCreateShader(type);
        _ERROR_RETURN_IF(shader == INVALID_ID, INVALID_ID, "GLUtil::loadShader create shader failed!\n%s", str);

        const char *cstr = str.c_str();
        glShaderSource(shader, 1, &cstr, nullptr);
        glCompileShader(shader);

        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (!compiled) {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

            if (infoLen > 1) {
                char * infoLog = new char[infoLen+1];
                memset(infoLog, 0, infoLen+1);
                glGetShaderInfoLog(shader, infoLen, nullptr, (GLchar *)infoLog);
                _ERROR("Error compiling shader:%s\n%s", infoLog, str);
                delete [] infoLog;
            }

            glDeleteShader(shader);
            return INVALID_ID;
        }

        return shader;
    }

    static GLuint loadProgram(const std::string &vstr, const std::string &fstr) {
        GLuint vertex = loadShader(vstr, GL_VERTEX_SHADER);
        _ERROR_RETURN_IF(vertex == INVALID_ID, INVALID_ID, "loadProgram vertex failed");

        GLuint fragment = loadShader(fstr, GL_FRAGMENT_SHADER);
        if (fragment == INVALID_ID) {
            _ERROR("loadProgram fragment failed");
            glDeleteShader(vertex);
            return INVALID_ID;
        }

        GLuint program = glCreateProgram();
        if (program == INVALID_ID) {
            _ERROR("loadProgram: create program error");
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            return INVALID_ID;
        }

        GLint linked;
        glAttachShader(program, vertex);
        glAttachShader(program, fragment);
        glLinkProgram(program);
        glGetProgramiv(program, GL_LINK_STATUS, &linked);

        if (!linked) {
            _ERROR("loadProgram linked error");
            GLint infoLen = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen > 1) {
                char * infoLog = new char[infoLen+1];
                memset(infoLog, 0, infoLen+1);
                glGetProgramInfoLog(program, infoLen, nullptr, (GLchar *)infoLog);
                _ERROR("Error linking program: %s", infoLog);
                delete [] infoLog;
            }
            glDeleteShader(vertex);
            glDeleteShader(fragment);
            glDeleteProgram(program);
            return INVALID_ID;
        }
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return program;
    }

    static void clearColor(float r, float g, float b, float a) {
        glClearColor(r, g, b, a);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    
    static GLsync createFenceSync() {
        GLsync gLsync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        _ERROR_RETURN_IF(gLsync == nullptr, nullptr, "GLUtil::createFenceSync failed");
        return gLsync;
    }
    
    static bool waitFenceSync(GLsync syncObj, GLbitfield flags = GL_SYNC_FLUSH_COMMANDS_BIT) {
        GLenum result = glClientWaitSync(syncObj, flags, GL_TIMEOUT_IGNORED);
        glDeleteSync(syncObj);
        if (result == GL_WAIT_FAILED) {
            _ERROR("GLUtil::waitFenceSync failed");
            return false;
        } else if (result == GL_TIMEOUT_EXPIRED) {
            _ERROR("GLUtil::waitFenceSync timeout");
            return false;
        }
        return true;
    }
    
    static void deleteFenceSync(GLsync syncObj) {
        glDeleteSync(syncObj);
    }

    static std::string glVendor() {
        return (const char *)glGetString(GL_VENDOR);
    }

    static std::string glRenderer() {
        return (const char *)glGetString(GL_RENDERER);
    }

    static std::string glExtensions() {
        return (const char *)glGetString(GL_EXTENSIONS);
    }

    static void checkGLError() {
        GLenum error = glGetError();
        if (error != GL_NO_ERROR) {
            _ERROR("GLUtil::checkGLError error: %s", errorString(error));
        }
    }

    static std::string errorString(GLenum error) {
        switch (error) {
        case GL_NO_ERROR:
            return "GL_NO_ERROR";
        case GL_INVALID_ENUM:
            return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE:
            return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION:
            return "GL_INVALID_OPERATION";
        case GL_OUT_OF_MEMORY:
            return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            return "GL_INVALID_FRAMEBUFFER_OPERATION";
        default:
            return "GL_ERROR_" + std::to_string(error);
        }
    }

};
NAMESPACE_END
