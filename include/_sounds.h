#ifndef _SOUNDS_H
#define _SOUNDS_H
#include <SNDS/irrKlang.h>

using namespace irrklang;
using namespace std;



class _sounds
{
    public:
        _sounds();
        virtual ~_sounds();

        void playMusic(const char* ); //Playing background music
        void playSFX(const char *, float volume = 1.0f); // Playing sound effects
        void pauseSounds(); // Pausing sound effects
        void initSounds();

        ISoundEngine *sndEng = createIrrKlangDevice();// creates sounds instance \


    protected:

    private:
};

#endif // _SOUNDS_H
