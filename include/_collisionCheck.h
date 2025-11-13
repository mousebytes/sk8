#ifndef _COLLISIONCHECK_H
#define _COLLISIONCHECK_H

#include<_common.h>

class _collisionCheck
{
    public:
        _collisionCheck();
        virtual ~_collisionCheck();

        bool isLinearCol(vec3,vec3);
        bool isRadialCol(vec2,vec2,float,float,float);//positions x,y , radius A,B, threshold
        bool isSphereCol(vec3,vec3,float,float,float);//positions x,y , radius A,B, threshold
        bool isPlanoCol(vec2,vec2);
        bool isCubicCol(vec3,vec3);


    protected:

    private:
};

#endif // _COLLISIONCHECK_H
