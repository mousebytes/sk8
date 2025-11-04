#ifndef _LIGHT_H
#define _LIGHT_H

#include<_common.h>

class _light
{
    public:
        _light();
        virtual ~_light();

        void setLight(GLenum);

        const float light_ambient[4] = {0,0,0,1};
        const float light_diffuse[4] = {1.0,1.0,1.0,1.0};
        const float light_specular[4]= {1.0,1.0,1.0,1.0};

        const float light_position[4]= {2.0,5.0,5.0,0.0};

        const float mat_ambient[4] = {0.7,0.7,0.7,1.0};
        const float mat_diffuse[4] = {0.8,0.8,0.8,1.0};
        const float mat_specular[4]= {1.0,1.0,1.0,1.0};
        const float high_shininess[2]= {100,0.0};

    protected:

    private:
};

#endif // _LIGHT_H
