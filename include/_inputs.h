#ifndef _INPUTS_H
#define _INPUTS_H

#include<_common.h>
#include<_model.h>
#include<_parallax.h>
#include<_skybox.h>
#include<_sprite.h>
#include<_camera.h>
#include<_3dmodelloader.h>
#include"_StaticModelInstance.h"


class _inputs
{
    public:
        _inputs();
        virtual ~_inputs();

        void keyPressed(_model *);    // key pressed on keyboard
        void keyPressed(_parallax *); // key pressed on keyboard
        void keyPressed(_skyBox *);
        void keyPressed(_sprite *);
        void keyPressed(_camera *);     // key pressed on keyboard
        void keyPressed(_3DModelLoader *,_3DModelLoader*);
        void keyPressed(_StaticModelInstance *staticModel);
        void keyUp(_sprite *);     // key released on keyboard

        void keyUp();              // key released on keyboard

        void mouseEventDown(_model *, double,double);//mouse pressed
        void mouseEventUp();        // mouse released

        void mouseWheel(_model*,double);// mouse wheel
        void mouseMove (_model*,double,double);//mouse move

        double prev_MouseX;  // keep track of mouse x position
        double prev_MouseY;  // keep track of mouse y position

        bool isRotationActive;// performing rotation
        bool isTranslateActive;//perform translation

        WPARAM wParam;
        LPARAM lParam;
    protected:

    private:
};

#endif // _INPUTS_H
