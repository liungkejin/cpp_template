//
// Created on 2024/7/9.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".


#pragma once
#include <znative/ZBase.h>
#include <cstdint>
#include <chrono>
#include <thread>
#include <iomanip>

NAMESPACE_DEFAULT

class TimeUtils {
public:
    static int64_t nowMs() {
        return std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    static int64_t nowUs() {
        return std::chrono::duration_cast<std::chrono::microseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
            .count();
    }

    static int64_t nowNs() {
        return std::chrono::duration_cast<std::chrono::nanoseconds>(
                   std::chrono::system_clock::now().time_since_epoch())
           .count();
    }

    static int64_t nowS() {
        return std::chrono::duration_cast<std::chrono::seconds>(
                   std::chrono::system_clock::now().time_since_epoch())
          .count();
    }

    /**
     * 解析时间字符串为时间戳，单位是毫秒
     * @param time_str 时间字符串，例如 "2024-07-09 14:20:30"
     * @param fmt 时间字符串的格式，默认是 "%Y-%m-%d %H:%M:%S"
     * @return 时间戳，单位是毫秒
     */
    static int64_t parseTime(const std::string& time_str, const std::string& fmt = "%Y-%m-%d %H:%M:%S") {
        std::tm tm = {};
        std::istringstream ss(time_str);
        ss >> std::get_time(&tm, fmt.c_str());
        if (ss.fail()) {
            return 0;
        }
        return std::mktime(&tm) * 1000;
    }

    static std::string formatMs(int64_t ms, const std::string& fmt = "%Y-%m-%d %H:%M:%S") {
        // 将毫秒转换为时间点
        auto time_point = std::chrono::system_clock::from_time_t(ms/1000);
        // 转换为系统时间
        auto system_time = std::chrono::system_clock::to_time_t(time_point);
        return format(system_time, fmt);
    }

    static std::string formatNow(const std::string& fmt = "%Y-%m-%d %H:%M:%S") {
        // 获取当前系统时间
        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        return format(now_time, fmt);
    }

    // 输出 2026-02-24 14:20:30
    static std::string format(std::time_t now_time, const std::string& fmt = "%Y-%m-%d %H:%M:%S") {
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

    /**
     * @return 当前时区相对UTC的偏移分钟数
     */
    static int currentZoneMinutes() {
        time_t now = time(nullptr);
        tm gmt = *gmtime(&now); // UTC时间
        tm local = *localtime(&now); // 本地时间

        time_t gmt_time = mktime(&gmt);
        int offset_minutes = static_cast<int>((now - gmt_time) / 60);
        return offset_minutes;
    }

    /**
     * @return 当前时区相对UTC的偏移小时数
     */
    static int currentZone() {
        return currentZoneMinutes() / 60;
    }
    
    static void sleepMs(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }
};

class CostMeter {
public:
    CostMeter() : m_start_us(TimeUtils::nowUs()) {}

    void reset() { m_start_us = TimeUtils::nowUs(); }

    int64_t costUs() const { return TimeUtils::nowUs() - m_start_us; }

    int64_t averageUs(int count) {
        if (m_count >= count) {
            m_count = 0;
            m_sum_cost_us = 0;
        }
        m_sum_cost_us += costUs();
        m_count++;
        return m_sum_cost_us / m_count;
    }
private:
    int64_t m_start_us = 0;
    int m_count = 0;
    int m_sum_cost_us = 0;
};

class FPSCounter {
public:
    explicit FPSCounter(int intervalMs = 1500) : m_interval(intervalMs) {}

    bool count() {
        bool updated = false;
        if (m_count == 0) {
            m_start = TimeUtils::nowMs();
        } else {
            int64_t end = TimeUtils::nowMs();
            int64_t duration = end - m_start;
            if (duration > m_interval) {
                m_fps = (float) ((double)m_count*1000 / (double)duration);
                m_start = end;
                m_count = 0;
                updated = true;
            }
        }
        m_count++;
        return updated;
    }

    inline float fps() const { return m_fps; }

private:
    int64_t m_start = 0;
    int m_count = 0;
    float m_fps = 0.0f;
    int m_interval = 1000;
};

NAMESPACE_END
