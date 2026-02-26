#pragma once

#include <vector>
#include <string>
#include <random>

// 确保 miniaudio.h 在你的 include 路径中
// 如果找不到，请改为 "miniaudio.h" 或相对路径
#include <miniaudio.h> 

class AudioSystem {
public:
    AudioSystem();
    ~AudioSystem();

    void PlayRain();
    void Update(bool isMoving, float deltaTime);

private:
    ma_engine engine;
    ma_sound rainSound;

    // 使用 vector 存储声音对象
    std::vector<ma_sound> footstepSounds;

    float footstepTimer;
    float footstepInterval;

    // 随机数生成器作为成员变量
    std::mt19937 rng;

    void PlayRandomFootstep();
};