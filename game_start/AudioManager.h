#pragma once
#include <miniaudio.h>
#include <string>
#include <vector>
#include <mutex>
#include <memory>

class AudioManager {
public:
    AudioManager();
    ~AudioManager();

    bool init();
    void shutdown();

    // 播放音效：支持音量，自动清理
    void play_sfx(const std::string& filepath, float volume = 1.0f);

    // 背景音乐：单例，支持循环/暂停/音量
    bool play_bgm(const std::string& filepath, float volume = 0.5f);
    void stop_bgm();
    void pause_bgm(bool paused);
    void set_bgm_volume(float vol);
    
    // 每帧调用此函数以清理已播放完毕的 SFX内存
    void update(); 

private:
    ma_engine* engine = nullptr;
    ma_sound*  bgm_sound = nullptr;
    
    // 用于管理临时创建的 SFX 声音对象
    std::vector<ma_sound*> active_sfx;
    std::mutex sfx_mutex;

    bool is_initialized = false;
};