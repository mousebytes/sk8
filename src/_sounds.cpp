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
    sndEng->play2D(fileName, true);
}

void _sounds::playSFX(const char* fileName)
{
    if(sndEng)
    sndEng->play2D(fileName, false);
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
