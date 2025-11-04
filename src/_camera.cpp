#include "_camera.h"

_camera::_camera()
{
    //ctor
}

_camera::~_camera()
{
    //dtor
}

void _camera::camInit()
{
    eye.x=0;eye.y=0;eye.z=10;
    destination.x = 0; destination.y = 0; destination.z = -10;
    up.x = 0; up.y = 1; up.z=0;

    step = 0.5;

    distance = sqrt(pow(eye.x-destination.x,2)+pow(eye.y-destination.y,2)+pow(eye.z-destination.z,2));

    rotAngle.x = 0;
    rotAngle.y = 0;
}

void _camera::camReset()
{
    eye.x=0;eye.y=0;eye.z=10;
    destination.x = 0; destination.y = 0; destination.z = 0;
    up.x = 0; up.y = 1; up.z=0;

    step = 0.5;

    distance = sqrt(pow(eye.x-destination.x,2)+pow(eye.y-destination.y,2)+pow(eye.z-destination.z,2));

    rotAngle.x = 0;
    rotAngle.y = 0;
}

void _camera::rotateXY()
{
    eye.x = destination.x + distance*cos(rotAngle.y * PI/180.0)*sin(rotAngle.x*PI/180.0);
    eye.y = destination.y + distance*sin(rotAngle.y*PI/180.0);
    eye.z = destination.z + distance*cos(rotAngle.y*PI/180.0)*cos(rotAngle.x*PI/180.0);
}

void _camera::rotateUP()
{

}

void _camera::camMoveFdBd(int dir)
{
    eye.z+= dir * step; // if forward dir = 1 -- backward dir = -1
    destination.z+=step*dir;
}

void _camera::camMoveLtRt(int dir)
{
    eye.x+=step*dir;
    destination.x+=step*dir;
}

void _camera::setUpCam()
{
    gluLookAt(eye.x,eye.y,eye.z,
              destination.x,destination.y,destination.z,
              up.x,up.y,up.z);
}
