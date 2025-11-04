#include "_timer.h"

_timer::_timer()
{
    //ctor

    startTime = clock();
}

_timer::~_timer()
{
    //dtor
}
clock_t _timer::getTicks()
{
    return clock()-startTime;
}

void _timer::reset()
{
    startTime = clock();
}

void _timer::pauseTime()
{

}

void _timer::resumeTime()
{

}
