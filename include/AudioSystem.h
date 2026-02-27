#pragma once

#include <vector>
#include <string>
#include <random>
#include <miniaudio.h>
#include <glm/glm.hpp> // 引入 glm 用于处理向量

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    void PlayRain();

    void PlayFootstep(bool isRunning);

private:
    ma_engine engine;
    ma_sound rainSound;

    // [修改] 两个独立的音频池
    std::vector<ma_sound> walkSounds;
    std::vector<ma_sound> runSounds;

    std::mt19937 rng;
};