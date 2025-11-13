#ifndef _CAMERA_H
#define _CAMERA_H
#include<_common.h>


class _camera
{
    public:
        _camera();
        virtual ~_camera();

        Vector3 eye;  // eye position
        Vector3 des;  // where you are looking at
        Vector3 up;   // camera head orientation

        float step;    // camera speed
        vec2 rotAngle; // rotation of the camera [left/right/up/down]
        float distance;// eye to source

        float mouseSensitivity;

        Vector2 deltas;

        bool isFreeCam;

        enum{FORWARD,BACKWARD,LEFT,RIGHT};

        void camInit(); //initilize the camera
        void camReset();// reset to original settings

        void rotateXY(); // rotate around in xy directions
        void rotateUP(); // rotate up and down

        void camMoveFdBd(int dir); // cam move back and forth
        void camMoveLtRt(int dir); // cam move left and right

        void setUpCamera();  // set the cam using gluLookat
        
        // handle mouse movement
        void handleMouse(HWND hWnd, int mouseX, int mouseY, int centerX, int centerY);

        Vector2 GetDeltas();
    protected:

    private:
};

#endif // _CAMERA_H
