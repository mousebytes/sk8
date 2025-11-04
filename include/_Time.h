#ifndef _SK8_TIME_H
#define _SK8_TIME_H

#include"_common.h"


class _Time{
    public:
        // time (seconds) since last frame
        static float deltaTime;
        // time since game started
        static float totalTime;

        // call at start of game
        static void Init();
        // call once before other updates
        static void Update();
    private:
    _Time() {} // prevent instantiation

    static double lastTime;
    protected:
};

#endif
