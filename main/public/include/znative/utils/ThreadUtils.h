//
// Created by LiangKeJin on 2025/6/12.
//

#pragma once

#include <thread>
#if defined(_WIN32) || defined(_WIN64)
#include <processthreadsapi.h>
#else
#include <unistd.h>
#endif

#include <znative/ZBase.h>

NAMESPACE_DEFAULT

class ThreadUtils {
public:
    static void sleep(int ms) {
        std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    }

    static int64_t thisThreadId() noexcept {
#if defined(_WIN32) || defined(_WIN64)
        return ::GetCurrentThreadId();
#else
        // https://android.googlesource.com/platform/bionic/+/master/libc/bionic/gettid.cpp
        // ::gettid();
#if defined(SYS_gettid)
        return syscall(SYS_gettid); /* syscall(__NR_gettid) or syscall(SYS_gettid); */
#elif defined(__NR_gettid)
        return syscall(__NR_gettid);
#elif defined(__ANDROID__) || defined(__HARMONYOS__)
        return gettid();
#else
        /* https://elliotth.blogspot.com/2012/04/gettid-on-mac-os.html */
        uint64_t tid;
        pthread_threadid_np(nullptr, &tid);

        return static_cast<int64_t>(tid);
#endif
#endif
    }

    static bool isThread(int64_t tid) noexcept {
        return thisThreadId() == tid;
    }
};

NAMESPACE_END
