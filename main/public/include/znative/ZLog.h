//
// Created on 2024/4/15.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#pragma once

#include <functional>
#include <string>
#include <tinyformat.h>

#ifndef _ZLOG_TAG
#define _ZLOG_TAG "zzz_native"
#endif

#if defined(__HARMONYOS__) || defined(__OHOS__)
#include <hilog/log.h>

#undef LOG_DOMAIN
#define LOG_DOMAIN 0x100

#undef LOG_TAG
#define LOG_TAG _ZLOG_TAG

#define __LOG_DEBUG(msg) OH_LOG_DEBUG(LOG_APP, "%{public}s", msg);
#define __LOG_INFO(msg) OH_LOG_INFO(LOG_APP, "%{public}s", msg);

#define __LOG_WARN(msg) OH_LOG_WARN(LOG_APP, "%{public}s", msg);
#define __LOG_ERROR(msg) OH_LOG_ERROR(LOG_APP, "%{public}s", msg);
#define __LOG_FATAL(msg) OH_LOG_ERROR(LOG_APP, "%{public}s", msg);

#elif defined(__ANDROID__)
#include <android/log.h>

#define __LOG_DEBUG(msg) __android_log_print(ANDROID_LOG_DEBUG, _ZLOG_TAG, "%s", msg)
#define __LOG_INFO(msg)  __android_log_print(ANDROID_LOG_INFO, _ZLOG_TAG, "%s", msg)

#define __LOG_WARN(msg)  __android_log_print(ANDROID_LOG_WARN, _ZLOG_TAG, "%s", msg)
#define __LOG_ERROR(msg) __android_log_print(ANDROID_LOG_ERROR, _ZLOG_TAG, "%s", msg)
#define __LOG_FATAL(msg) __android_log_print(ANDROID_LOG_ERROR, _ZLOG_TAG, "%s", msg)

#elif defined(WIN32) || defined(_WIN32)

#define __LOG_DEBUG(msg) fprintf(stdout, "%s\n", msg);
#define __LOG_INFO(msg)  fprintf(stdout, "%s\n", msg);

#define __LOG_WARN(msg)  fprintf(stdout, "%s\n", msg);
#define __LOG_ERROR(msg) fprintf(stdout, "%s\n", msg);
#define __LOG_FATAL(msg) fprintf(stderr, "%s\n", msg);

#elif (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE == 1)

#define __LOG_DEBUG(msg) fprintf(stdout, "%s\n", msg);
#define __LOG_INFO(msg)  fprintf(stdout, "%s\n", msg);
#define __LOG_WARN(msg)  fprintf(stdout, "%s\n", msg);
#define __LOG_ERROR(msg) fprintf(stdout, "%s\n", msg);
#define __LOG_FATAL(msg) fprintf(stderr, "%s\n", msg);

#else

#define __LOG_DEBUG(msg) fprintf(stdout, "%s\n", msg);
#define __LOG_INFO(msg)  fprintf(stdout, "\033[32m%s\033[0m\n", msg);
#define __LOG_WARN(msg)  fprintf(stdout, "\033[33m%s\033[0m\n", msg);
#define __LOG_ERROR(msg) fprintf(stdout, "\033[31m%s\033[0m\n", msg);
#define __LOG_FATAL(msg) fprintf(stderr, "\033[31m%s\033[0m\n", msg);

#endif

enum ZLogLevel {
    LEVEL_DEBUG = 0,
    LEVEL_INFO = 1,
    LEVEL_WARN = 2,
    LEVEL_ERROR = 3,
    LEVEL_FATAL = 4
};

class ZLog {
public:
    static void setLevel(ZLogLevel level);

    static void setLogFile(const std::string& path);

    static void setStrictMode(bool strictMode);

    static bool isStrictMode();

    /**
     * @brief 日志回调函数，用于自定义日志输出
     * @param cb 参数 level, log 日志内容
     * @return true: 继续输出日志，false: 停止输出日志, 就是不会继续调用 printf 或者 写入文件
     */
    static void setLogCallback(const std::function<bool(ZLogLevel level, const std::string&)>& cb);

    static void print(ZLogLevel level, const std::string& msg);

    static int64_t curThreadId();

    static std::string prettyTimeTag();

    static std::string prettyMethodName(const std::string& prettyFunction);
};

#if defined(__ANDROID__) || defined(__HARMONYOS__)
#define __PRETTY_FORMAT(fmt, ...)                                                                                      \
    tfm::format("[T:%5lld][%s:%04d] " fmt, ZLog::curThreadId(), ZLog::prettyMethodName(__PRETTY_FUNCTION__), __LINE__, ##__VA_ARGS__)
#elif defined(_MSC_VER)
#define __PRETTY_FORMAT(fmt, ...)                                                                                      \
    tfm::format("[%s] [T:%5lld][%s:%04d] " fmt, ZLog::prettyTimeTag(),  ZLog::curThreadId(), ZLog::prettyMethodName(__FUNCTION__), __LINE__, ##__VA_ARGS__)
#else
#define __PRETTY_FORMAT(fmt, ...)                                                                                      \
    tfm::format("[%s] [T:%5lld][%s:%04d] " fmt, ZLog::prettyTimeTag(), ZLog::curThreadId(), ZLog::prettyMethodName(__PRETTY_FUNCTION__), __LINE__, ##__VA_ARGS__)
#endif


#define _PRINT(fmt, ...)                                                                                               \
    do {                                                                                                               \
        std::string _log_str = __PRETTY_FORMAT(fmt, ##__VA_ARGS__);                                                    \
        ZLog::print(LEVEL_DEBUG, _log_str);                                                                            \
    } while (0)

#define _INFO(fmt, ...)                                                                                                \
    do {                                                                                                               \
        std::string _log_str = __PRETTY_FORMAT(fmt, ##__VA_ARGS__);                                                    \
        ZLog::print(LEVEL_INFO, _log_str);                                                                             \
    } while (0)

#define _WARN(fmt, ...)                                                                                                \
    do {                                                                                                               \
        std::string _log_str = __PRETTY_FORMAT(fmt, ##__VA_ARGS__);                                                    \
        ZLog::print(LEVEL_WARN, _log_str);                                                                             \
    } while (0)

#define _ERROR(fmt, ...)                                                                                               \
    do {                                                                                                               \
        std::string _log_str = __PRETTY_FORMAT(fmt, ##__VA_ARGS__);                                                    \
        ZLog::print(LEVEL_ERROR, _log_str);                                                                            \
        if (ZLog::isStrictMode()) {                                                                                    \
            throw std::runtime_error(_log_str);                                                                        \
        }                                                                                                              \
    } while (0)

#define _FATAL(fmt, ...)                                                                                               \
    do {                                                                                                               \
        std::string _log_str = __PRETTY_FORMAT(fmt, ##__VA_ARGS__);                                                    \
        ZLog::print(LEVEL_FATAL, _log_str);                                                                            \
        throw std::runtime_error(_log_str);                                                                            \
    } while (0)

#define _FATAL_IF(condition, fmt, ...)                                                                                 \
    if (condition) {                                                                                                   \
        _FATAL(fmt, ##__VA_ARGS__);                                                                                    \
    }

#define _ERROR_RETURN_IF(condition, retcode, fmt, ...)                                                                 \
    if (condition) {                                                                                                   \
        _ERROR(fmt, ##__VA_ARGS__);                                                                                    \
        return (retcode);                                                                                              \
    }

#define _ERROR_IF(condition, fmt, ...)                                                                                 \
    if (condition) {                                                                                                   \
        _ERROR(fmt, ##__VA_ARGS__);                                                                                    \
    }

#define _WARN_RETURN_IF(condition, retcode, fmt, ...)                                                                  \
    if (condition) {                                                                                                   \
        _WARN(fmt, ##__VA_ARGS__);                                                                                     \
        return (retcode);                                                                                              \
    }

#define _WARN_IF(condition, fmt, ...)                                                                                  \
    if (condition) {                                                                                                   \
        _WARN(fmt, ##__VA_ARGS__);                                                                                     \
    }

#define _INFO_IF(condition, fmt, ...)                                                                                  \
    if (condition) {                                                                                                   \
        _INFO(fmt, ##__VA_ARGS__);                                                                                     \
    }

#define _CHECK_RESULT(error, fmt, ...)                                                                                 \
    if (error) {                                                                                                       \
        _ERROR("Error(%d): " #fmt, error, ##__VA_ARGS__);                                                              \
    } else {                                                                                                           \
        _INFO(fmt, __VA_ARGS__);                                                                                       \
    }

#define _CHECK_FUNC(result, func)                                                                                      \
    auto result = func;                                                                                                \
    if (result) {                                                                                                      \
        _ERROR("" #func " failed: %d", result);                                                                        \
    } else {                                                                                                           \
        _INFO("" #func " success");                                                                                    \
    }
