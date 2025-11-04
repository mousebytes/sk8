#include"_Time.h"


// initialize static vars
float _Time::deltaTime = 0.0f;
float _Time::totalTime = 0.0f;

// simplify this ugly thing
using Clock = std::chrono::high_resolution_clock;

static Clock::time_point g_GameStartTime;
static Clock::time_point g_LastFrameTime;

void _Time::Init(){
    // get cur time when game starts

    g_GameStartTime=Clock::now();
    g_LastFrameTime=g_GameStartTime;

    deltaTime=0.0f;
    totalTime=0.0f;
}

void _Time::Update(){
    //get cur time
    Clock::time_point currentTime = Clock::now();

    // calc time diff since last frame
    std::chrono::duration<float> frameDuration=currentTime-g_LastFrameTime;
    // calc time diff since game started
    std::chrono::duration<float> totalDuration = currentTime - g_GameStartTime;

    // store duration (seconds)

    deltaTime = frameDuration.count();
    totalTime=totalDuration.count();

    // update last frame time
    g_LastFrameTime=currentTime;
}
