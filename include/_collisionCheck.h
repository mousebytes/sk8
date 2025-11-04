#ifndef _COLLISIONCHECK_H
#define _COLLISIONCHECK_H

#include<_common.h>

class _collisionCheck
{
    public:
        _collisionCheck();
        virtual ~_collisionCheck();

        bool isLinearCol(vec3,vec3);
        // pos1, pos2, rad1, rad2, threshold
        bool isRadialCol(vec2,vec2,float,float,float);
        // pos1, pos2, rad1, rad2, threshold
        bool isSphereCol(vec3,vec3,float,float,float);
        bool isPlanarCol(vec2,vec2);
        bool isCubicCol(vec3,vec3);

    protected:

    private:
};

#endif // _COLLISIONCHECK_H
