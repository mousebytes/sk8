#ifndef _SPRITE_H
#define _SPRITE_H

#include<_common.h>
#include<_textureloader.h>


class _sprite
{
    public:
        _sprite();
        virtual ~_sprite();

        _textureLoader *myTex = new _textureLoader();

        vec3 pos;
        vec3 rot;
        vec3 pSize;

        vec2 frames;
        vec3 vertX[4];

        float xMin,yMin,xMax,yMax;

        void spriteInit(char *, int, int);// filename, # of frames x & y
        void drawSprite();
        void spriteActions();

        enum{STAND,WALKLEFT,WALKRIGHT,WALKFRONT,WALKBACK};

        int actionTrigger;

    protected:

    private:
};

#endif // _SPRITE_H
