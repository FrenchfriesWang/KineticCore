#define MINIAUDIO_IMPLEMENTATION
#include "AudioSystem.h"
#include <iostream>

AudioSystem::AudioSystem() : footstepTimer(0.0f), footstepInterval(0.45f) {
    // 1. 初始化引擎
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        std::cerr << "Audio Engine Init Failed!" << std::endl;
        return;
    }

    // 2. 加载背景雨声 (循环)
    if (ma_sound_init_from_file(&engine, "assets/audio/rainy.wav", 0, NULL, NULL, &rainSound) == MA_SUCCESS) {
        ma_sound_set_looping(&rainSound, MA_TRUE);
    }
    else {
        std::cerr << "Failed to load rainy.ogg" << std::endl;
    }

    // 3. 加载脚步声 (致命Bug修复版)
    // 必须先分配内存，确保地址不会变动，否则 miniaudio 会崩溃
    footstepSounds.resize(10);

    for (int i = 0; i < 10; ++i) {
        // 拼凑文件名 footstep_1.wav 到 footstep_10.wav
        std::string path = "assets/audio/footstep_" + std::to_string(i + 1) + ".wav";

        // 直接在 vector 分配好的内存上初始化，严禁拷贝
        // 移除 ASYNC 标志，确保进游戏就能立刻播放
        ma_result result = ma_sound_init_from_file(&engine, path.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &footstepSounds[i]);

        if (result != MA_SUCCESS) {
            std::cerr << "Failed to load " << path << std::endl;
        }
    }

    // 4. 初始化随机数生成器
    std::random_device rd;
    rng = std::mt19937(rd());
}

AudioSystem::~AudioSystem() {
    ma_sound_uninit(&rainSound);
    // 释放所有脚步声
    for (auto& sound : footstepSounds) {
        ma_sound_uninit(&sound);
    }
    ma_engine_uninit(&engine);
}

void AudioSystem::PlayRain() {
    // 加上检测，防止加载失败时调用崩溃
    if (ma_sound_is_playing(&rainSound)) return;
    ma_sound_start(&rainSound);
}

void AudioSystem::Update(bool isMoving, float deltaTime) {
    if (isMoving) {
        footstepTimer -= deltaTime;
        if (footstepTimer <= 0.0f) {
            PlayRandomFootstep();
            footstepTimer = footstepInterval;
        }
    }
    else {
        // 【核心修复】：不要直接粗暴清零！
        // 让计时器即使在停下时，也按照真实时间自然衰减。
        // 这样就算玩家像抽筋一样狂按 ADAD，只要 0.45 秒的冷却没走完，就绝对按不出第二声。
        if (footstepTimer > 0.0f) {
            footstepTimer -= deltaTime;
        }
        else {
            // 只有当冷却真正结束了（比如彻底停下站了半秒钟），才保持在 0。
            // 保证下一次起步依然能立刻发声。
            footstepTimer = 0.0f;
        }
    }
}

void AudioSystem::PlayRandomFootstep() {
    if (footstepSounds.empty()) return;

    // 均匀分布随机数 0 ~ 9
    std::uniform_int_distribution<> dis(0, static_cast<int>(footstepSounds.size()) - 1);
    int index = dis(rng);

    // 重置并播放
    ma_sound_seek_to_pcm_frame(&footstepSounds[index], 0);
    ma_sound_start(&footstepSounds[index]);
}