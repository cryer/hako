#include <windows.h>
#include <chrono>
#include <thread>
#include <immintrin.h>
#include <iostream>

class FrameLimiter {
    LARGE_INTEGER freq_;
    double targetFrameTime_;
    double lastFrameTime_;

public:
    explicit FrameLimiter(int targetFPS) : targetFrameTime_(1.0 / targetFPS) {
        QueryPerformanceFrequency(&freq_);
        LARGE_INTEGER t; 
        QueryPerformanceCounter(&t);
        lastFrameTime_ = t.QuadPart / (double)freq_.QuadPart;

        // 将系统定时器分辨率提升至 1ms（突破默认的 15.6ms 限制）
        timeBeginPeriod(1);
    }

    ~FrameLimiter() {
        timeEndPeriod(1); // 程序退出时恢复默认，避免影响其他进程
    }

    void wait() {
        LARGE_INTEGER t;
        QueryPerformanceCounter(&t);
        double currentTime = t.QuadPart / (double)freq_.QuadPart;
        double elapsed = currentTime - lastFrameTime_;
        double sleepTime = targetFrameTime_ - elapsed;

        if (sleepTime > 0.002) { // 仅当剩余时间 > 2ms 时才休眠
            // 1. 粗调：休眠到剩余约 1ms
            DWORD sleepMs = static_cast<DWORD>(sleepTime * 1000) - 1;
            if (sleepMs > 0) Sleep(sleepMs);

            // 2. 精调：最后 1ms 使用高精度自旋等待
            while (true) {
                QueryPerformanceCounter(&t);
                currentTime = t.QuadPart / (double)freq_.QuadPart;
                elapsed = currentTime - lastFrameTime_;
                if (elapsed >= targetFrameTime_) break;
                _mm_pause(); // CPU 节能指令，降低自旋功耗与发热
            }
        }
        // 使用实际结束时间作为下一帧基准，避免误差累积
        lastFrameTime_ = currentTime;
    }
};