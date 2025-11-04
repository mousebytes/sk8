#ifndef _CAMERA_H
#define _CAMERA_H

#include<_common.h>

class _camera
{
    public:
        _camera();
        virtual ~_camera();

        vec3 eye; // eye position
        vec3 destination; // where you're looking at
        vec3 up; // camera head orientation

        float step; // camera speed
        vec3 rotAngle; // rotation of the camera (left,right,up,down)
        float distance; // eye to source

        enum {FORWARD,BACKWARD,LEFT,RIGHT};

        void camInit(); // initialize the camera
        void camReset(); // reset to original settings

        void rotateXY(); // rotate around XY directions
        void rotateUP(); // rotate up and down

        void camMoveFdBd(int dir); // cam move forward and backward
        void camMoveLtRt(int dir); // cam move left and right

        void setUpCam(); // set the cam using gluLookAt

    protected:

    private:
};

#endif // _CAMERA_H
