#define MINIAUDIO_IMPLEMENTATION
#include "AudioSystem.h"
#include <iostream>

AudioSystem::AudioSystem() : lastStepPos(0.0f, 0.0f), isFirstStep(true), stepDistance(0.85f) { // 1.5f 暂定为一个步伐的距离
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
        std::cerr << "Failed to load rainy.wav" << std::endl;
    }

    // 3. 加载脚步声 
    footstepSounds.resize(10);
    for (int i = 0; i < 10; ++i) {
        std::string path = "assets/audio/footstep_" + std::to_string(i + 1) + ".wav";
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

void AudioSystem::Update(glm::vec2 currentPos) {
    // 初始化：进游戏的第一帧，把锚点打在脚下
    if (isFirstStep) {
        lastStepPos = currentPos;
        isFirstStep = false;
        return;
    }

    // 计算当前位置与上一个脚印的绝对直线距离
    float distance = glm::distance(currentPos, lastStepPos);

    // 只有真真切切地走出了这个半径（无论你怎么ADAD绕圈子），才触发声音
    if (distance >= stepDistance) {
        PlayRandomFootstep();
        lastStepPos = currentPos; // 在新位置重新打下锚点
    }
}

void AudioSystem::PlayRandomFootstep() {
    if (footstepSounds.empty()) return;

    // 【删除】绝对不要再用 ma_sound_stop 去打断前一个声音了！
    // 即使频率变快，也让 miniaudio 自然叠加“脚跟-脚尖”，这比强行掐断要自然得多。

    std::uniform_int_distribution<> dis(0, static_cast<int>(footstepSounds.size()) - 1);
    int index = dis(rng);

    ma_sound_seek_to_pcm_frame(&footstepSounds[index], 0);
    ma_sound_start(&footstepSounds[index]);
}