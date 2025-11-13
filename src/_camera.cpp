#include "_camera.h"

_camera::_camera()
{
    //ctor
    isFreeCam=false;
}

_camera::~_camera()
{
    //dtor
}
void _camera::camInit()
{
    // Default to a 3rd person view
    eye.x =0; eye.y =5; eye.z =10;  // Start behind and above
    des.x =0; des.y =0; des.z =0;   // Look at the origin
    up.x  =0; up.y  =1; up.z  =0;

    // since i'm using delta time now
    // this is how far to move per second
    step =30;

    // distance is now controlled by _Player, but set a default
    distance = 10.0;

    rotAngle.x =0;  // Yaw
    rotAngle.y =20; // Pitch
    mouseSensitivity = 0.1f;
}

void _camera::camReset()
{
    // Reset to a 3rd person view
    eye.x =0; eye.y =5; eye.z =10;
    des.x =0; des.y =0; des.z =0;
    up.x  =0; up.y  =1; up.z  =0;

    step =30;

    distance = 10.0;

    rotAngle.x =0;
    rotAngle.y =20;
    mouseSensitivity = 0.1f;
}

void _camera::rotateXY()
{
    // This logic is for orbiting the *destination* (des)
    // It's used by free-cam
    eye.x = des.x + distance*cos(rotAngle.y*PI/180.0)*sin(rotAngle.x*PI/180.0);
    eye.y = des.y + distance*sin(rotAngle.y*PI/180.0);
    eye.z = des.z + distance*cos(rotAngle.y*PI/180.0)*cos(rotAngle.x*PI/180.0);
}

void _camera::rotateUP()
{

}

void _camera::camMoveFdBd(int dir)
{
    if(!isFreeCam) return;
    float moveStep = step * _Time::deltaTime;
    
    // calc the "forward" direction vector on the XZ plane
    // this is based on the camera's yaw (rotAngle.x)
    // (des - eye) gives the forward vector
    float fwdX = -sin(rotAngle.x * PI / 180.0);
    float fwdZ = -cos(rotAngle.x * PI / 180.0);
    // 1 for forward (W) and -1 for backwards (S)
    float moveAmount = moveStep * dir;
    // apply movement
    eye.x += fwdX * moveAmount;
    eye.z += fwdZ * moveAmount;
    des.x += fwdX * moveAmount;
    des.z += fwdZ * moveAmount;
}

void _camera::camMoveLtRt(int dir)
{
    if(!isFreeCam) return;

    float moveStep = step * _Time::deltaTime;

    // calc the "right" direction vector (strafe)
    // this is the forward vector rotated 90 degrees
    float rightX = cos(rotAngle.x * PI / 180.0);
    float rightZ = -sin(rotAngle.x * PI / 180.0);

    // -1 for left (A) 1 for right (D)
    float moveAmount = moveStep * dir;

    // apply the strafe movement 
    eye.x += rightX * moveAmount;
    eye.z += rightZ * moveAmount;
    
    des.x += rightX * moveAmount;
    des.z += rightZ * moveAmount;
}

void _camera::setUpCamera()
{
    gluLookAt(eye.x,eye.y,eye.z,
              des.x,des.y,des.z,
              up.x, up.y, up.z);
}

void _camera::handleMouse(HWND hWnd, int mouseX, int mouseY, int centerX, int centerY){
    // This function is ONLY for the FREE-CAM
    if(!isFreeCam) return;

    // if the mouse is at the center it's because we just set it
    // in the last frame, ignore this "fake" event
    if (mouseX == centerX && mouseY == centerY) {
        return;
    }

    // calc delta from the center
    deltas.x = (float)(mouseX - centerX);
    deltas.y = (float)(mouseY - centerY);

    // apply to camera rotation
    rotAngle.x -= deltas.x * mouseSensitivity;
    
    // invert y axis (screen y is down, camera up is up)
    //rotAngle.y -= deltas.y * mouseSensitivity;
    rotAngle.y +=deltas.y*mouseSensitivity;

    // clamp vertical rotation
    // we use 89.0 to avoid gimbal lock at 90.0
    if(rotAngle.y > 89.0f) {
        rotAngle.y = 89.0f;
    }
    if(rotAngle.y < -89.0f) {
        rotAngle.y = -89.0f;
    }

    // update the camera's internal state
    rotateXY();

    // reset mouse to center
    // convert the window's center to the screen's coordinate system
    POINT centerPoint = { centerX, centerY };
    ClientToScreen(hWnd, &centerPoint);
    SetCursorPos(centerPoint.x, centerPoint.y);
}

Vector2 _camera::GetDeltas(){
    return deltas;
}