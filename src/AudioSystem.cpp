#define MINIAUDIO_IMPLEMENTATION
#include "AudioSystem.h"
#include <iostream>

AudioSystem::AudioSystem() {
    if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
        return;
    }

    if (ma_sound_init_from_file(&engine, "assets/audio/rainy.wav", 0, NULL, NULL, &rainSound) == MA_SUCCESS) {
        ma_sound_set_looping(&rainSound, MA_TRUE);
    }

    // [核心修改] 分别预分配内存并加载 10 份 Walk 和 10 份 Run
    walkSounds.resize(10);
    runSounds.resize(10);
    for (int i = 0; i < 10; ++i) {
        std::string walkPath = "assets/audio/walk_" + std::to_string(i + 1) + ".wav";
        ma_sound_init_from_file(&engine, walkPath.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &walkSounds[i]);

        std::string runPath = "assets/audio/run_" + std::to_string(i + 1) + ".wav";
        ma_sound_init_from_file(&engine, runPath.c_str(), MA_SOUND_FLAG_DECODE, NULL, NULL, &runSounds[i]);
    }

    std::random_device rd;
    rng = std::mt19937(rd());
}

AudioSystem::~AudioSystem() {
    ma_sound_uninit(&rainSound);
    for (auto& sound : walkSounds) ma_sound_uninit(&sound);
    for (auto& sound : runSounds) ma_sound_uninit(&sound);
    ma_engine_uninit(&engine);
}

void AudioSystem::PlayRain() {
    // 加上检测，防止加载失败时调用崩溃
    if (ma_sound_is_playing(&rainSound)) return;
    ma_sound_start(&rainSound);
}


void AudioSystem::PlayFootstep(bool isRunning) {
    // 拿到对应的池子引用
    std::vector<ma_sound>& targetPool = isRunning ? runSounds : walkSounds;

    if (targetPool.empty()) return;

    std::uniform_int_distribution<> dis(0, static_cast<int>(targetPool.size()) - 1);
    int index = dis(rng);

    ma_sound_seek_to_pcm_frame(&targetPool[index], 0);
    ma_sound_start(&targetPool[index]);
}