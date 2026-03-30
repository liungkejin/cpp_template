//
// Created on 2024/4/15.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#include <iomanip>
#include <iostream>
#include <sstream>
#include <tinyformat.h>
#include <znative/ZLog.h>
#include <znative/utils/ThreadUtils.h>
#include <znative/utils/TimeUtils.h>

static ZLogLevel __g_level = ZLogLevel::LEVEL_INFO;

FILE *__g_logFile = nullptr;
bool __g_strictMode = false;
std::function<bool(ZLogLevel level, const std::string&)> __g_logCallback = nullptr;

void ZLog::setLogFile(const std::string& path) {
    if (__g_logFile != nullptr) {
        fclose(__g_logFile);
        __g_logFile = nullptr;
    }
    __g_logFile = fopen(path.c_str(), "a+");
    if (__g_logFile == nullptr) {
        _ERROR("open log file failed: %s", path);
    }
}

void ZLog::setLevel(ZLogLevel level) {
    __g_level = level;
}

void ZLog::setLogCallback(const std::function<bool(ZLogLevel level, const std::string&)>& cb) {
    __g_logCallback = cb;
}

void ZLog::setStrictMode(bool strictMode) {
    __g_strictMode = strictMode;
}

bool ZLog::isStrictMode() {
    return __g_strictMode;
}

static std::string __formatNow(const std::string& fmt = "%Y-%m-%d %H:%M:%S") {
    // 获取当前系统时间
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    // 转换为本地时间
    std::tm local_tm;

    // 跨平台时间转换（Windows 使用 localtime_s，其他平台使用 localtime_r）
#ifdef _WIN32
    localtime_s(&local_tm, &now_time);
#else
    localtime_r(&now_time, &local_tm);
#endif

    // 格式化输出
    std::stringstream ss;
    ss << std::put_time(&local_tm, fmt.c_str());
    return ss.str();
}

int64_t ZLog::curThreadId() {
    return znative::ThreadUtils::thisThreadId();
}

std::string ZLog::prettyTimeTag() {
    int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
        .count();
    int millisecond = nowMs % 1000;
    char timeTag[30];
    std::string nowStr = __formatNow("%m-%d %H:%M:%S");
    snprintf(timeTag, sizeof(timeTag), "%s.%03d", nowStr.c_str(), millisecond);
    return timeTag;
}

std::string ZLog::prettyMethodName(const std::string& prettyFunction) {
    size_t end = prettyFunction.find('(');
    size_t beg = prettyFunction.rfind(' ', end);
    std::string methodName = prettyFunction.substr(beg + 1, end - beg - 1);
    int maxLen = 35;
    int len = methodName.length();
    if (len > maxLen) {
        methodName = methodName.substr(len - maxLen, maxLen) + "()";
    }
    else {
        methodName = methodName + "()" + std::string(maxLen - methodName.length(), ' ');
    }
    return methodName;
}

void ZLog::print(ZLogLevel level, const std::string& msg) {
    if (__g_level > level) {
        return;
    }
    if (__g_logCallback && !__g_logCallback(level, msg)) {
        return;
    }
    if (__g_logFile != nullptr) {
        std::string timetag;
#if defined(__ANDROID__) || defined(__HARMONYOS__) || defined(__IOS__) || defined(TARGET_OS_IPHONE)
        timetag = "[" + prettyTimeTag() + "] ";
#endif
        if (level == LEVEL_DEBUG) {
            fprintf(__g_logFile, "[D] %s%s\n", timetag.c_str(), msg.c_str());
        } else if (level == LEVEL_INFO) {
            fprintf(__g_logFile, "[I] %s%s\n", timetag.c_str(), msg.c_str());
        } else if (level == LEVEL_ERROR) {
            fprintf(__g_logFile, "[E] %s%s\n", timetag.c_str(), msg.c_str());
        } else if (level == LEVEL_WARN) {
            fprintf(__g_logFile, "[W] %s%s\n", timetag.c_str(), msg.c_str());
        } else if (level == LEVEL_FATAL) {
            fprintf(__g_logFile, "[F] %s%s\n", timetag.c_str(), msg.c_str());
        } else {
            fprintf(__g_logFile, "[U] %s%s\n", timetag.c_str(), msg.c_str());
        }
        fflush(__g_logFile);
    } else {
        if (level == LEVEL_DEBUG) {
            __LOG_DEBUG(msg.c_str());
        } else if (level == LEVEL_INFO) {
            __LOG_INFO(msg.c_str());
        } else if (level == LEVEL_ERROR) {
            __LOG_ERROR(msg.c_str());
        } else if (level == LEVEL_WARN) {
            __LOG_WARN(msg.c_str());
        } else if (level == LEVEL_FATAL) {
            __LOG_FATAL(msg.c_str());
        }
    }
}


