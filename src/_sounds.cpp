#include "_sounds.h"
#include "_common.h"

_sounds::_sounds()
{
    //ctor
    sndEng = NULL;

}

_sounds::~_sounds()
{
    //dtor
    if(sndEng)
        sndEng->drop();
}
void _sounds::playMusic(const char* fileName)
{
    if(sndEng)
    {
      // Play 2D, loop = true, startPaused = false, track = true
      ISound* music = sndEng->play2D(fileName, true, false, true);

      if(music)
      {
          music->setVolume(0.25f); // Lowered background music volume
          music->drop();
      }
    }
}

void _sounds::playSFX(const char* fileName, float volume)
{
    if(sndEng)
    {
        // Play 2D, loop = false, startPaused = false, track = true
        ISound* sfx = sndEng->play2D(fileName, false, false, true);

        if (sfx)
        {
            sfx->setVolume(volume);
            sfx->drop();
        }
    }
}

void _sounds::pauseSounds()
{
    if(sndEng)
    sndEng->stopAllSounds();
}

void _sounds::initSounds()
{
    sndEng = createIrrKlangDevice();
    if(!sndEng)
    {
        std::cout << "Error: Sound Engine Failed!" << endl;

    }

}
