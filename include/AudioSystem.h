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

    // [修改] 直接传入玩家当前的 XZ 坐标
    void Update(glm::vec2 currentPos);

private:
    ma_engine engine;
    ma_sound rainSound;
    std::vector<ma_sound> footstepSounds;

    // [核心修改] 锚点法所需变量
    glm::vec2 lastStepPos;
    bool isFirstStep;
    float stepDistance;

    std::mt19937 rng;
    void PlayRandomFootstep();
};