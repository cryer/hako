#define MINIAUDIO_IMPLEMENTATION
#include "AudioManager.h"
#include <iostream>
#include <algorithm>
#include <mutex>

AudioManager::AudioManager() : engine(nullptr), bgm_sound(nullptr), is_initialized(false) {}

AudioManager::~AudioManager() {
    shutdown();
}

bool AudioManager::init() {
    if (is_initialized) return true;

    engine = new ma_engine();
    // 使用默认配置初始化引擎
    ma_result result = ma_engine_init(nullptr, engine);
    
    if (result != MA_SUCCESS) {
        std::cerr << "[Audio] Init failed: " << ma_result_description(result) << "\n";
        delete engine;
        engine = nullptr;
        return false;
    }

    is_initialized = true;
    std::cout << "[Audio] Engine initialized.\n";
    return true;
}

void AudioManager::shutdown() {
    if (!is_initialized || !engine) return;

    stop_bgm();
    
    // 清理所有残留的 SFX
    {
        std::lock_guard<std::mutex> lock(sfx_mutex);
        for (auto* s : active_sfx) {
            ma_sound_uninit(s);
            delete s;
        }
        active_sfx.clear();
    }

    ma_engine_uninit(engine);
    delete engine;
    engine = nullptr;
    is_initialized = false;
}

void AudioManager::play_sfx(const std::string& filepath, float volume) {
    if (!is_initialized || !engine) return;

    ma_sound* sfx = new ma_sound();
    
    // 初始化声音对象
    // 参数说明: 
    // pEngine: 引擎指针
    // pFilePath: 文件路径
    // flags: 0 (默认加载到内存，适合短音效)
    // pAllocationCallbacks: nullptr (默认分配器)
    // pSoundGroup: nullptr (无分组)
    // pSound: 输出指针
    ma_result res = ma_sound_init_from_file(engine, filepath.c_str(), 0, nullptr, nullptr, sfx);
    
    if (res != MA_SUCCESS) {
        std::cerr << "[Audio] SFX load failed (" << filepath << "): " << ma_result_description(res) << "\n";
        delete sfx;
        return;
    }

    ma_sound_set_volume(sfx, volume);
    ma_sound_start(sfx);

    //加入活跃列表
    std::lock_guard<std::mutex> lock(sfx_mutex);
    active_sfx.push_back(sfx);
}

bool AudioManager::play_bgm(const std::string& filepath, float volume) {
    if (!is_initialized || !engine) return false;

    // 停止旧的 BGM
    stop_bgm();

    bgm_sound = new ma_sound();
    
    // 初始化 BGM
    // 注意：这里 flags 传 0，表示加载到内存。如果文件很大，可以传 MA_SOUND_FLAG_STREAM
    ma_result res = ma_sound_init_from_file(engine, filepath.c_str(), 0, nullptr, nullptr, bgm_sound);
    
    if (res != MA_SUCCESS) {
        std::cerr << "[Audio] BGM load failed: " << ma_result_description(res) << "\n";
        delete bgm_sound;
        bgm_sound = nullptr;
        return false;
    }

    ma_sound_set_looping(bgm_sound, MA_TRUE);
    
    ma_sound_set_volume(bgm_sound, volume);
    ma_sound_start(bgm_sound);
    
    return true;
}

void AudioManager::stop_bgm() {
    if (bgm_sound) {
        ma_sound_stop(bgm_sound);
        ma_sound_uninit(bgm_sound);
        delete bgm_sound;
        bgm_sound = nullptr;
    }
}

void AudioManager::pause_bgm(bool paused) {
    if (bgm_sound) {
        if (paused) {
            ma_sound_stop(bgm_sound);
        } else {
            ma_sound_start(bgm_sound);
        }
    }
}

void AudioManager::set_bgm_volume(float vol) {
    if (bgm_sound) {
        ma_sound_set_volume(bgm_sound, vol);
    }
}

void AudioManager::update() {
    if (!is_initialized) return;

    std::lock_guard<std::mutex> lock(sfx_mutex);
    
    // 逆向遍历或 erase-remove idiom 清理已结束的音效
    for (auto it = active_sfx.begin(); it != active_sfx.end(); ) {
        ma_sound* s = *it;
        
        // 检查是否还在播放
        if (!ma_sound_is_playing(s)) {
            ma_sound_uninit(s);
            delete s;
            it = active_sfx.erase(it);
        } else {
            ++it;
        }
    }
}